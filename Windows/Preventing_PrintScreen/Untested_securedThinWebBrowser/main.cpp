#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <wrl.h>
#include <wrl/event.h>

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <sstream>
#include <memory>
#include <algorithm>
#include <cwctype>

#include "WebView2.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Make;

namespace {

struct AppConfig {
    std::wstring scheme = L"http";
    std::wstring host;
    int port = 0;
    std::wstring context;
    std::vector<std::pair<std::wstring, std::wstring>> params;
    std::wstring title = L"Doxis webCube Wrapper";
    std::wstring userDataDir;
    std::wstring directUrl;
    bool showHelp = false;
};

struct AppState {
    AppConfig config;
    HWND hwnd = nullptr;
    HMODULE webView2Loader = nullptr;
    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> webview;
};

AppState g_app;

using CreateCoreWebView2EnvironmentWithOptionsFn = HRESULT (STDAPICALLTYPE*)(
    PCWSTR,
    PCWSTR,
    ICoreWebView2EnvironmentOptions*,
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);

std::wstring TrimSlashes(const std::wstring& value) {
    size_t start = 0;
    while (start < value.size() && (value[start] == L'/' || value[start] == L'\\')) {
        ++start;
    }
    size_t end = value.size();
    while (end > start && (value[end - 1] == L'/' || value[end - 1] == L'\\')) {
        --end;
    }
    return value.substr(start, end - start);
}

bool IsUnreserved(unsigned char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '-' || c == '.' || c == '_' || c == '~';
}

std::string WideToUtf8(const std::wstring& value) {
    if (value.empty()) {
        return {};
    }
    int bytes = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    std::string out(bytes, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), out.data(), bytes, nullptr, nullptr);
    return out;
}

std::wstring Utf8ToWide(const std::string& value) {
    if (value.empty()) {
        return {};
    }
    int chars = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), nullptr, 0);
    std::wstring out(chars, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), out.data(), chars);
    return out;
}

std::wstring PercentEncode(const std::wstring& input, bool keepSlash) {
    static const char* hex = "0123456789ABCDEF";
    std::string utf8 = WideToUtf8(input);
    std::string out;
    out.reserve(utf8.size() * 3);

    for (unsigned char c : utf8) {
        if (IsUnreserved(c) || (keepSlash && c == '/')) {
            out.push_back(static_cast<char>(c));
        } else {
            out.push_back('%');
            out.push_back(hex[(c >> 4) & 0xF]);
            out.push_back(hex[c & 0xF]);
        }
    }

    return Utf8ToWide(out);
}

std::wstring GetLocalAppDataPath() {
    PWSTR raw = nullptr;
    std::wstring path;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &raw)) && raw) {
        path = raw;
    }
    if (raw) {
        CoTaskMemFree(raw);
    }
    return path;
}

std::wstring GetDefaultUserDataDir() {
    std::wstring base = GetLocalAppDataPath();
    if (base.empty()) {
        return L".\\DoxisWebCubeWrapper\\WebView2";
    }
    return base + L"\\DoxisWebCubeWrapper\\WebView2";
}

void EnsureDirectoryExists(const std::wstring& path) {
    if (path.empty()) {
        return;
    }
    SHCreateDirectoryExW(nullptr, path.c_str(), nullptr);
}

std::wstring BuildUsageText() {
    return
        L"Usage:\n\n"
        L"  DoxisWebCubeWrapper.exe --host <host> --context <context> [options]\n"
        L"  DoxisWebCubeWrapper.exe --url <full-url> [options]\n\n"
        L"Required for structured mode:\n"
        L"  --host <host>           Example: appserver.example.local\n"
        L"  --context <context>     Example: documents/12345 or searches/ABCDEF\n\n"
        L"Optional:\n"
        L"  --scheme <http|https>   Default: http\n"
        L"  --port <n>              Example: 8080\n"
        L"  --param <k=v>           Repeatable. Example: --param view=versionlist\n"
        L"  --title <text>          Window title\n"
        L"  --user-data-dir <path>  Persistent WebView2 profile directory\n"
        L"  --url <full-url>        Bypass structured URL construction\n"
        L"  --help                  Show this message\n\n"
        L"Structured URL format built by the app:\n"
        L"  <scheme>://<host>[:port]/webcube/<context>?<params>\n";
}

void ShowUsage() {
    MessageBoxW(nullptr, BuildUsageText().c_str(), L"Doxis webCube Wrapper", MB_OK | MB_ICONINFORMATION);
}

std::optional<std::wstring> GetArgValue(int argc, wchar_t** argv, int& i) {
    if (i + 1 >= argc) {
        return std::nullopt;
    }
    ++i;
    return std::wstring(argv[i]);
}

bool ParseParam(const std::wstring& raw, std::pair<std::wstring, std::wstring>& out) {
    size_t pos = raw.find(L'=');
    if (pos == std::wstring::npos || pos == 0) {
        return false;
    }
    out.first = raw.substr(0, pos);
    out.second = raw.substr(pos + 1);
    return true;
}

bool StartsWithInsensitive(const std::wstring& value, const std::wstring& prefix) {
    if (value.size() < prefix.size()) {
        return false;
    }
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (towlower(value[i]) != towlower(prefix[i])) {
            return false;
        }
    }
    return true;
}

bool ParseCommandLine(AppConfig& config) {
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        return false;
    }

    std::unique_ptr<wchar_t*, decltype(&LocalFree)> argvGuard(argv, &LocalFree);

    for (int i = 1; i < argc; ++i) {
        std::wstring arg = argv[i];
        if (arg == L"--help" || arg == L"-h" || arg == L"/?") {
            config.showHelp = true;
            return true;
        } else if (arg == L"--scheme") {
            auto value = GetArgValue(argc, argv, i);
            if (!value) return false;
            config.scheme = *value;
        } else if (arg == L"--host") {
            auto value = GetArgValue(argc, argv, i);
            if (!value) return false;
            config.host = *value;
        } else if (arg == L"--port") {
            auto value = GetArgValue(argc, argv, i);
            if (!value) return false;
            config.port = _wtoi(value->c_str());
        } else if (arg == L"--context") {
            auto value = GetArgValue(argc, argv, i);
            if (!value) return false;
            config.context = *value;
        } else if (arg == L"--param") {
            auto value = GetArgValue(argc, argv, i);
            if (!value) return false;
            std::pair<std::wstring, std::wstring> kv;
            if (!ParseParam(*value, kv)) {
                return false;
            }
            config.params.push_back(std::move(kv));
        } else if (arg == L"--title") {
            auto value = GetArgValue(argc, argv, i);
            if (!value) return false;
            config.title = *value;
        } else if (arg == L"--user-data-dir") {
            auto value = GetArgValue(argc, argv, i);
            if (!value) return false;
            config.userDataDir = *value;
        } else if (arg == L"--url") {
            auto value = GetArgValue(argc, argv, i);
            if (!value) return false;
            config.directUrl = *value;
        } else {
            return false;
        }
    }

    if (config.userDataDir.empty()) {
        config.userDataDir = GetDefaultUserDataDir();
    }

    if (!config.directUrl.empty()) {
        return true;
    }

    return !config.host.empty() && !config.context.empty();
}

std::wstring BuildTargetUrl(const AppConfig& config) {
    if (!config.directUrl.empty()) {
        return config.directUrl;
    }

    std::wstring context = TrimSlashes(config.context);
    if (StartsWithInsensitive(context, L"webcube/")) {
        context = context.substr(8);
    }

    std::wstringstream url;
    url << config.scheme << L"://" << config.host;
    if (config.port > 0) {
        url << L":" << config.port;
    }
    url << L"/webcube/" << PercentEncode(context, true);

    if (!config.params.empty()) {
        url << L"?";
        for (size_t i = 0; i < config.params.size(); ++i) {
            if (i > 0) {
                url << L"&";
            }
            url << PercentEncode(config.params[i].first, false)
                << L"="
                << PercentEncode(config.params[i].second, false);
        }
    }

    return url.str();
}

std::wstring HResultMessage(HRESULT hr) {
    wchar_t* buffer = nullptr;
    DWORD size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        static_cast<DWORD>(hr),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer),
        0,
        nullptr);

    std::wstring text;
    if (size && buffer) {
        text.assign(buffer, size);
        LocalFree(buffer);
    } else {
        std::wstringstream ss;
        ss << L"HRESULT 0x" << std::hex << hr;
        text = ss.str();
    }
    return text;
}

void FailAndClose(const std::wstring& text) {
    MessageBoxW(g_app.hwnd, text.c_str(), L"Doxis webCube Wrapper", MB_OK | MB_ICONERROR);
    if (g_app.hwnd) {
        DestroyWindow(g_app.hwnd);
    }
}

void ResizeWebViewToClientArea() {
    if (!g_app.controller || !g_app.hwnd) {
        return;
    }
    RECT bounds{};
    GetClientRect(g_app.hwnd, &bounds);
    g_app.controller->put_Bounds(bounds);
}

void ApplyBrowserSettings() {
    if (!g_app.webview) {
        return;
    }

    ComPtr<ICoreWebView2Settings> settings;
    if (FAILED(g_app.webview->get_Settings(&settings)) || !settings) {
        return;
    }

    settings->put_AreDevToolsEnabled(FALSE);
    settings->put_IsZoomControlEnabled(FALSE);

    ComPtr<ICoreWebView2Settings2> settings2;
    if (SUCCEEDED(settings.As(&settings2)) && settings2) {
        settings2->put_AreDefaultContextMenusEnabled(FALSE);
        settings2->put_IsStatusBarEnabled(FALSE);
    }

    ComPtr<ICoreWebView2Settings3> settings3;
    if (SUCCEEDED(settings.As(&settings3)) && settings3) {
        settings3->put_AreBrowserAcceleratorKeysEnabled(FALSE);
    }
}

void AttachEvents() {
    if (!g_app.webview) {
        return;
    }

    EventRegistrationToken token{};

    g_app.webview->add_NewWindowRequested(
        Callback<ICoreWebView2NewWindowRequestedEventHandler>(
            [](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
                LPWSTR uri = nullptr;
                if (SUCCEEDED(args->get_Uri(&uri)) && uri && sender) {
                    sender->Navigate(uri);
                }
                if (uri) {
                    CoTaskMemFree(uri);
                }
                args->put_Handled(TRUE);
                return S_OK;
            }).Get(),
        &token);

    g_app.webview->add_WindowCloseRequested(
        Callback<ICoreWebView2WindowCloseRequestedEventHandler>(
            [](ICoreWebView2*, IUnknown*) -> HRESULT {
                if (g_app.hwnd) {
                    PostMessageW(g_app.hwnd, WM_CLOSE, 0, 0);
                }
                return S_OK;
            }).Get(),
        &token);

    g_app.webview->add_NavigationCompleted(
        Callback<ICoreWebView2NavigationCompletedEventHandler>(
            [](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                BOOL success = FALSE;
                if (SUCCEEDED(args->get_IsSuccess(&success)) && !success) {
                    COREWEBVIEW2_WEB_ERROR_STATUS status{};
                    args->get_WebErrorStatus(&status);
                }
                return S_OK;
            }).Get(),
        &token);
}

bool EnableCaptureProtection(HWND hwnd) {
    return SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE) != FALSE;
}

HRESULT CreateWebView() {
    if (g_app.webView2Loader == nullptr) {
        g_app.webView2Loader = LoadLibraryW(L"WebView2Loader.dll");
        if (!g_app.webView2Loader) {
            FailAndClose(L"WebView2Loader.dll was not found next to the executable or on the DLL search path.");
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    auto createEnvironment = reinterpret_cast<CreateCoreWebView2EnvironmentWithOptionsFn>(
        GetProcAddress(g_app.webView2Loader, "CreateCoreWebView2EnvironmentWithOptions"));
    if (!createEnvironment) {
        FailAndClose(L"CreateCoreWebView2EnvironmentWithOptions could not be loaded from WebView2Loader.dll.");
        return HRESULT_FROM_WIN32(GetLastError());
    }

    EnsureDirectoryExists(g_app.config.userDataDir);

    auto options = Make<CoreWebView2EnvironmentOptions>();
    if (!options) {
        return E_OUTOFMEMORY;
    }
    options->put_AllowSingleSignOnUsingOSPrimaryAccount(TRUE);

    return createEnvironment(
        nullptr,
        g_app.config.userDataDir.c_str(),
        options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
                if (FAILED(result) || !environment) {
                    FailAndClose(std::wstring(L"Failed to create WebView2 environment:\n\n") + HResultMessage(result));
                    return result;
                }

                environment->CreateCoreWebView2Controller(
                    g_app.hwnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (FAILED(result) || !controller) {
                                FailAndClose(std::wstring(L"Failed to create WebView2 controller:\n\n") + HResultMessage(result));
                                return result;
                            }

                            g_app.controller = controller;
                            g_app.controller->get_CoreWebView2(&g_app.webview);
                            if (!g_app.webview) {
                                FailAndClose(L"WebView2 controller was created but CoreWebView2 is null.");
                                return E_FAIL;
                            }

                            ApplyBrowserSettings();
                            AttachEvents();
                            ResizeWebViewToClientArea();
                            g_app.controller->put_IsVisible(TRUE);

                            const std::wstring targetUrl = BuildTargetUrl(g_app.config);
                            g_app.webview->Navigate(targetUrl.c_str());
                            return S_OK;
                        }).Get());

                return S_OK;
            }).Get());
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        g_app.hwnd = hwnd;
        EnableCaptureProtection(hwnd);
        CreateWebView();
        return 0;

    case WM_SIZE:
        ResizeWebViewToClientArea();
        return 0;

    case WM_DPICHANGED: {
        const RECT* suggested = reinterpret_cast<const RECT*>(lParam);
        SetWindowPos(hwnd, nullptr,
            suggested->left,
            suggested->top,
            suggested->right - suggested->left,
            suggested->bottom - suggested->top,
            SWP_NOZORDER | SWP_NOACTIVATE);
        ResizeWebViewToClientArea();
        return 0;
    }

    case WM_DESTROY:
        g_app.webview.Reset();
        g_app.controller.Reset();
        if (g_app.webView2Loader) {
            FreeLibrary(g_app.webView2Loader);
            g_app.webView2Loader = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    AppConfig config;
    if (!ParseCommandLine(config)) {
        ShowUsage();
        return 2;
    }
    if (config.showHelp) {
        ShowUsage();
        return 0;
    }

    if (config.title.empty()) {
        config.title = L"Doxis webCube Wrapper";
    }

    g_app.config = std::move(config);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, (std::wstring(L"CoInitializeEx failed:\n\n") + HResultMessage(hr)).c_str(), L"Doxis webCube Wrapper", MB_OK | MB_ICONERROR);
        return 1;
    }

    const wchar_t kWindowClassName[] = L"DoxisWebCubeWrapperWindow";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = kWindowClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(nullptr, L"RegisterClassExW failed.", L"Doxis webCube Wrapper", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return 1;
    }

    HWND hwnd = CreateWindowExW(
        0,
        kWindowClassName,
        g_app.config.title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1400,
        900,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hwnd) {
        MessageBoxW(nullptr, L"CreateWindowExW failed.", L"Doxis webCube Wrapper", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CoUninitialize();
    return static_cast<int>(msg.wParam);
}
