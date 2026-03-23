#include <windows.h>

#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

static HFONT g_font = nullptr;

static HFONT CreateAppFont(int pointSize) {
    HDC hdc = GetDC(nullptr);
    int logPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(nullptr, hdc);

    int height = -MulDiv(pointSize, logPixelsY, 72);
    return CreateFontW(
        height, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
}

LRESULT CALLBACK LeftProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HWND hLabel = CreateWindowW(
            L"STATIC",
            L"this is screenshotable",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            40, 90, 360, 40,
            hwnd,
            nullptr,
            ((LPCREATESTRUCT)lParam)->hInstance,
            nullptr
        );
        SendMessageW(hLabel, WM_SETFONT, (WPARAM)g_font, TRUE);
        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        static HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
        return (LRESULT)hBrush;
    }

    case WM_ERASEBKGND: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect((HDC)wParam, &rc, CreateSolidBrush(RGB(240, 240, 240)));
        return 1;
    }

    case WM_DESTROY:
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK RightProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

        HWND hLabel1 = CreateWindowW(
            L"STATIC",
            L"If you can screenshot this, the PoC failed.",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 60, 460, 40,
            hwnd,
            nullptr,
            hInst,
            nullptr
        );
        SendMessageW(hLabel1, WM_SETFONT, (WPARAM)g_font, TRUE);

        HWND hLabel2 = CreateWindowW(
            L"STATIC",
            L"If it does not, buy me an espresso brewed with Ospina Dynasty beans.",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 120, 460, 70,
            hwnd,
            nullptr,
            hInst,
            nullptr
        );
        SendMessageW(hLabel2, WM_SETFONT, (WPARAM)g_font, TRUE);

        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        static HBRUSH hBrush = CreateSolidBrush(RGB(30, 30, 30));
        return (LRESULT)hBrush;
    }

    case WM_ERASEBKGND: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect((HDC)wParam, &rc, CreateSolidBrush(RGB(30, 30, 30)));
        return 1;
    }

    case WM_DESTROY:
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    g_font = CreateAppFont(16);

    WNDCLASSW wc = {};
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    wc.lpfnWndProc = LeftProc;
    wc.lpszClassName = L"LeftPaneTopLevel";
    RegisterClassW(&wc);

    wc.lpfnWndProc = RightProc;
    wc.lpszClassName = L"RightPaneTopLevel";
    RegisterClassW(&wc);

    const int x = 100, y = 100, w = 520, h = 300;

    HWND g_left = CreateWindowExW(
        0,
        L"LeftPaneTopLevel",
        L"Screenshotable Pane",
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        nullptr, nullptr, hInstance, nullptr
    );

    HWND g_right = CreateWindowExW(
        0,
        L"RightPaneTopLevel",
        L"Protected Pane",
        WS_OVERLAPPEDWINDOW,
        x + w, y, w, h,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!g_left || !g_right) {
        MessageBoxW(nullptr, L"Failed to create windows.", L"Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(g_left, nCmdShow);
    ShowWindow(g_right, nCmdShow);
    UpdateWindow(g_left);
    UpdateWindow(g_right);

    if (!SetWindowDisplayAffinity(g_right, WDA_EXCLUDEFROMCAPTURE)) {
        DWORD err = GetLastError();
        wchar_t buf[128];
        wsprintfW(buf, L"SetWindowDisplayAffinity failed. Error: %lu", err);
        MessageBoxW(g_right, buf, L"Error", MB_ICONERROR);
    }

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_font) {
        DeleteObject(g_font);
        g_font = nullptr;
    }

    return 0;
}
