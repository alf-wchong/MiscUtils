# MFC Protected Pane PoC

This is a Visual Studio 2022 x64 MFC proof of concept for Windows 11 that shows two panes inside one MFC frame window:

- Left pane: normal GDI text, intentionally capturable.
- Right pane: DirectComposition + composition-swapchain `IPresentationSurface` with `SetDisableReadback(TRUE)`.

## Expected result

When you capture the window with Snipping Tool / Print Screen / Windows Graphics Capture, the left pane should be visible in the capture, while the right pane should appear black or absent depending on the capture path and OS behavior.

## Requirements

- Windows 11 build 22000.194 or later
- WDDM 2.0+ GPU driver
- Visual Studio 2022 with MFC installed
- Windows SDK new enough to provide `presentation.h`

## Notes

- The right pane is the protected pane. The user prompt appeared to say the second label was also in the left pane; this PoC interprets that as a typo and places the second label in the right pane.
- The neon blue border is implemented as a 5-pixel client-area border to keep the PoC simple and focused on pane-level capture blocking.
- The code is intended as a PoC and may need small SDK/version adjustments depending on your exact Windows SDK.
