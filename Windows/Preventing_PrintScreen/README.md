# PoC - Preventing Screenshots from capturing the contents of a Window

## Overview

This project is a minimal proof-of-concept (PoC) demonstrating how Windows applications can prevent their content from being captured by standard screenshot tools while remaining fully visible on the user’s monitor.

It uses the Windows API function:

[SetWindowDisplayAffinity](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity)(hwnd, [WDA_EXCLUDEFROMCAPTURE](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity#parameters))


## What This Demonstrates

The application creates **two side-by-side windows**:

### Left Window (Screenshotable)

* Normal top-level window
* Contains a label: `this is screenshotable`
* Appears normally in screenshots

### Right Window (Protected)

* Top-level window with display affinity set to `WDA_EXCLUDEFROMCAPTURE`
* Contains two labels:

  * "If you can screenshot this, the PoC failed."
  * "If it does not, buy me an espresso brewed with Ospina Dynasty beans."
* Fully visible on screen
* **Hidden or blank in standard screenshots** (Snipping Tool, Print Screen, etc.)

## Key Insight

Windows applies capture protection **per top-level window only**.

* ❌ You cannot protect individual controls or panes inside a window
* ✅ You *can* protect an entire window

This is why the PoC uses **two separate windows** instead of a split pane.

## Build Instructions (Fedora / Linux)

Install MinGW-w64:

```bash
sudo dnf install mingw64-gcc-c++
```

Compile:

```bash
x86_64-w64-mingw32-g++ -municode -O2 -static -static-libgcc -static-libstdc++ \
  -o poc2winpanels.exe poc2winpanels.cpp -lgdi32 -luser32
```

### Docker image alternate

Alternatively, use the [Dockerfile](Dockerfile) provided to spin up a docker image that has all the items needed to do this work. 

## Run Instructions

1. Copy `poc2winpanels.exe` to a Windows 10/11 machine
2. Run the executable
3. Try capturing the screen using:

   * Print Screen
   * Snipping Tool
   * Greenshot
   * OBS (window capture)

## Expected Behavior

| Window | On Screen | Screenshot     |
| ------ | --------- | -------------- |
| Left   | Visible   | Visible        |
| Right  | Visible   | Hidden / Blank |

## Requirements

* Windows 10 (2004+) or Windows 11
* Desktop Window Manager (DWM) enabled (default)

## Limitations

This technique is **not a security boundary**.

It does NOT prevent:

* Taking photos of the screen
* Some advanced capture methods
* Certain remote desktop or virtualization scenarios

Microsoft considers this a **best-effort privacy feature**, not DRM.

## Notes

* Calling `SetWindowDisplayAffinity` must be done on a **top-level window**
* Calling it on child windows results in:

  * `ERROR_INVALID_PARAMETER (87)`
* It should be called **after the window is created and shown**

## Purpose

This PoC is intended for:

* Demonstrating Windows capture behavior
* Testing environments where screenshot prevention is desired

