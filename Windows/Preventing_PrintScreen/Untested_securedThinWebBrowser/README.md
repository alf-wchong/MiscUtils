# Screen capture/sharing disabled viewer for Doxis managed documents (Win32 + WebView2)

This prototype is a **single-file native Windows desktop wrapper** around a modern web app.

It uses **Win32 + Microsoft Edge WebView2**, because the target web app ([HTML5 viewer](https://services.sergroup.com/documentation/#/view/PD_webCube/14.3.0/en-us/UG_Doxis_webCube/WEBHELP/APP_webCube/topics/top_DocumentDisplay_HTML5Viewer_Intro.html) component of [Doxis webCube](https://services.sergroup.com/documentation/#/view/PD_webCube/14.3.0/en-us/UG_Doxis_webCube/WEBHELP/APP_webCube/topics/top_ChapIntro_IntroductionWebCube.html)) is modern and needs a current browser engine.

## What it does

- Creates a normal resizable framed desktop window.
- Hosts the external Doxis-style web app inside a single WebView2 control.
- Applies `SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE)` to the top-level frame window.
- Accepts **structured command-line arguments** and builds URLs in this form:

```text
<scheme>://<host>[:port]/webcube/<context>?<params>
```

- Disables most browser chrome-like affordances:
  - DevTools
  - default context menu
  - browser accelerator keys
  - zoom control
  - status bar
- Redirects `window.open(...)` / new-window requests back into the same embedded view.
- Persists the WebView2 profile under `%LOCALAPPDATA%\DoxisWebCubeWrapper\WebView2` unless overridden.

## Important cookie / sign-in note

This wrapper enables **OS-account single sign-on** for WebView2 and uses a persistent WebView2 user-data folder so sessions survive across runs.

However, WebView2 still maintains its **own** browser profile data for the app. In practice that means this prototype is good for **persistent wrapper-local sign-in**, but you should **not assume full reuse of the exact cookie jar from the user’s normal browser profile**.

If you need stricter SSO behavior for a specific enterprise identity flow, test that flow explicitly against your target Doxis deployment.

## Files

- `main.cpp` — the entire prototype.

## Recommended toolchain

Use **MSVC / Visual Studio 2022**.

Why: WebView2’s Win32 SDK and callback helpers are much smoother with MSVC than with MinGW, and this prototype is written to optimize for the Windows host you will actually test on.

## Build prerequisites

1. **Visual Studio 2022** with **Desktop development with C++**.
2. A recent **Windows SDK**.
3. The **WebView2 SDK** headers and loader DLL.
   - The easiest path is to download the `Microsoft.Web.WebView2` NuGet package and extract it.
   - You need:
     - `build\native\include\WebView2.h`
     - `build\native\x64\WebView2Loader.dll`
4. A Windows 10/11 machine with the **WebView2 Runtime** installed.

## Suggested folder layout

```text
DoxisWebCubeWrapper/
  main.cpp
  README.md
  webview2-sdk/
    build/
      native/
        include/
          WebView2.h
        x64/
          WebView2Loader.dll
```

## Build from an x64 Native Tools Command Prompt

```bat
cl /nologo /std:c++17 /EHsc /DUNICODE /D_UNICODE /W4 /MD main.cpp /I webview2-sdk\build\native\include /link user32.lib ole32.lib shell32.lib shlwapi.lib /SUBSYSTEM:WINDOWS /OUT:DoxisWebCubeWrapper.exe
```

Then copy the loader DLL next to the executable:

```bat
copy webview2-sdk\build\native\x64\WebView2Loader.dll .
```

Result:

```text
DoxisWebCubeWrapper.exe
WebView2Loader.dll
```

## Run examples

### Structured Doxis-style URL

```bat
DoxisWebCubeWrapper.exe ^
  --scheme http ^
  --host appserver01.example.local ^
  --port 8080 ^
  --context documents/123456789 ^
  --param view=versionlist ^
  --title "Doxis document viewer"
```

This produces:

```text
http://appserver01.example.local:8080/webcube/documents/123456789?view=versionlist
```

### Multiple parameters

```bat
DoxisWebCubeWrapper.exe ^
  --scheme http ^
  --host appserver01.example.local ^
  --port 8080 ^
  --context searches/ABCDEF0123456789 ^
  --param foo=bar ^
  --param view=export ^
  --title "Doxis export view"
```

### Full URL passthrough

```bat
DoxisWebCubeWrapper.exe --url "http://appserver01.example.local:8080/webcube/documents/123456789?view=versionlist"
```

### Custom WebView2 user-data directory

```bat
DoxisWebCubeWrapper.exe ^
  --scheme https ^
  --host doxis.example.com ^
  --context documents/123456789 ^
  --user-data-dir "C:\Temp\DoxisWrapperProfile"
```

## Command-line reference

```text
--host <host>           Required in structured mode
--context <context>     Required in structured mode
--scheme <http|https>   Optional, defaults to http
--port <n>              Optional
--param <k=v>           Optional, repeatable
--title <text>          Optional window title
--user-data-dir <path>  Optional persistent WebView2 profile directory
--url <full-url>        Optional direct full URL mode
--help                  Show usage
```

## Behavior notes

- `context` should normally be relative to `/webcube/`, for example:
  - `documents/<id>`
  - `searches/<id>`
  - `workbaskets/<id>`
- If `context` is passed as `webcube/<something>`, the wrapper normalizes it to avoid a doubled `/webcube/webcube/...` path.
- Query parameter names and values are percent-encoded by the wrapper.
- Path context is percent-encoded while preserving `/` path separators.
- The top-level window is resizable and uses the standard Windows frame.
- There are no native navigation controls; navigation is driven by the web app itself.

## Troubleshooting

### “WebView2Loader.dll was not found”

Place the correct architecture-specific `WebView2Loader.dll` next to the EXE.

### Blank or broken login flow

Check whether the target environment expects browser cookies or SSO behavior that differs from a dedicated WebView2 profile.

### Screen capture protection does not behave as expected

`SetWindowDisplayAffinity` only applies to the top-level window and depends on Windows support for `WDA_EXCLUDEFROMCAPTURE`.

### Need stricter lockdown

This prototype keeps the app simple. If you later want kiosk-style restrictions, consider adding:

- navigation allow-listing
- download blocking
- permission filtering
- script injection for additional host-side policy
- single-instance profile locking rules

## Next sensible enhancements

- Add a small JSON config file option.
- Add host/path allow-listing.
- Add structured logging to a file.
- Add optional custom headers or pre-auth hooks.
- Add enterprise certificate / auth diagnostics.
