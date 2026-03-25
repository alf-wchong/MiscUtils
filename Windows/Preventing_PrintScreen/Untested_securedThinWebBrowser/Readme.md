Build it on **Windows with MSVC / Visual Studio 2022**.

### 1. Install prerequisites

On the Windows host, install:

* **Visual Studio 2022** with **Desktop development with C++**
* A recent **Windows SDK**
* The **WebView2 Runtime**
* The **WebView2 SDK** headers and loader DLL from the `Microsoft.Web.WebView2` NuGet package

### 2. Put the files in a folder

Use a layout like this:

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

### 3. Open the right command prompt

Open:

```text
x64 Native Tools Command Prompt for VS 2022
```

Then `cd` into the project folder.

### 4. Compile

Run:

```bat
cl /nologo /std:c++17 /EHsc /DUNICODE /D_UNICODE /W4 /MD main.cpp /I webview2-sdk\build\native\include /link user32.lib ole32.lib shell32.lib shlwapi.lib /SUBSYSTEM:WINDOWS /OUT:DoxisWebCubeWrapper.exe
```

### 5. Copy the WebView2 loader DLL next to the EXE

Run:

```bat
copy webview2-sdk\build\native\x64\WebView2Loader.dll .
```

You should then have:

```text
DoxisWebCubeWrapper.exe
WebView2Loader.dll
```

### 6. Run it

Example:

```bat
DoxisWebCubeWrapper.exe ^
  --scheme http ^
  --host appserver01.example.local ^
  --port 8080 ^
  --context documents/123456789 ^
  --param view=versionlist ^
  --title "Doxis document viewer"
```

Or with a full URL:

```bat
DoxisWebCubeWrapper.exe --url "http://appserver01.example.local:8080/webcube/documents/123456789?view=versionlist"
```

### Notes

* The code expects `WebView2Loader.dll` to be **next to the executable** or otherwise on the DLL search path.
* If you get a message that `WebView2Loader.dll` was not found, that DLL is either missing or the wrong architecture.
