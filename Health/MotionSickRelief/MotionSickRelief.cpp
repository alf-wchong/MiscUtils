#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <vector>
#include <cmath>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")

const int SAMPLE_RATE = 44100;
const int FREQUENCY = 100;
const double PI = 3.141592653589793;

HWND hButtons[5], hBtnStop, hSlider;
HWAVEOUT hWaveOut = NULL;
WAVEHDR waveHdr;
bool isPlaying = false;
UINT_PTR timerId = 0, drawTimerId = 0;
int totalSeconds = 0, elapsedSeconds = 0;

void SetUIState(HWND hwnd, bool playing) {
    for (int i = 0; i < 5; i++) EnableWindow(hButtons[i], !playing);
    EnableWindow(hBtnStop, playing);
    isPlaying = playing;
    InvalidateRect(hwnd, NULL, TRUE); 
}

void StopAudio(HWND hwnd, bool closeApp) {
    if (!isPlaying) return;
    KillTimer(hwnd, timerId);
    KillTimer(hwnd, drawTimerId);
    if (hWaveOut) {
        waveOutReset(hWaveOut);
        waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
        waveOutClose(hWaveOut);
        hWaveOut = NULL;
    }
    if (closeApp) PostQuitMessage(0);
    else SetUIState(hwnd, false);
}

void PlayTone(HWND hwnd, int seconds) {
    totalSeconds = seconds;
    elapsedSeconds = 0;
    
    // Generate Buffer
    size_t bufSize = SAMPLE_RATE * seconds;
    static std::vector<short> audioBuffer;
    audioBuffer.assign(bufSize, 0);
    for (size_t i = 0; i < bufSize; ++i) {
        audioBuffer[i] = (short)(15000 * sin(2 * PI * FREQUENCY * i / SAMPLE_RATE));
    }

    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, SAMPLE_RATE, SAMPLE_RATE * 2, 2, 16, 0 };
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) return;

    waveHdr = { (LPSTR)audioBuffer.data(), (DWORD)(bufSize * 2), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

    // Initial Volume from Slider
    int pos = SendMessage(hSlider, TBM_GETPOS, 0, 0);
    DWORD vol = (pos * 0xFFFF / 100);
    waveOutSetVolume(hWaveOut, MAKELONG(vol, vol));

    SetUIState(hwnd, true);
    timerId = SetTimer(hwnd, 1, seconds * 1000, NULL);
    drawTimerId = SetTimer(hwnd, 2, 100, NULL); // Refresh UI every 100ms
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_BAR_CLASSES };
        InitCommonControlsEx(&icex);
        
        const wchar_t* labels[] = { L"1min", L"3min", L"5min", L"10min", L"30min" };
        int vals[] = { 1, 3, 5, 10, 30 };

        for (int i = 0; i < 5; i++) {
            hButtons[i] = CreateWindow(L"BUTTON", labels[i], WS_VISIBLE | WS_CHILD, 
                            10 + (i * 75), 10, 70, 30, hwnd, 
                            (HMENU)(UINT_PTR)vals[i], // Fixes the size warning
                            NULL, NULL);
        }
        
        CreateWindow(L"STATIC", L"Volume:", WS_VISIBLE | WS_CHILD, 10, 55, 60, 20, hwnd, NULL, NULL, NULL);
        hSlider = CreateWindow(TRACKBAR_CLASS, L"Volume", WS_VISIBLE | WS_CHILD | TBS_AUTOTICKS, 
                               70, 50, 200, 30, hwnd, (HMENU)100, NULL, NULL);
        SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
        SendMessage(hSlider, TBM_SETPOS, TRUE, 50);

        hBtnStop = CreateWindow(L"BUTTON", L"STOP", WS_VISIBLE | WS_CHILD | WS_DISABLED, 
                                300, 50, 70, 30, hwnd, (HMENU)99, NULL, NULL);
        return 0;
    }
	
    case WM_HSCROLL: {
        if (hWaveOut) {
            int pos = SendMessage(hSlider, TBM_GETPOS, 0, 0);
            DWORD vol = (pos * 0xFFFF / 100);
            waveOutSetVolume(hWaveOut, MAKELONG(vol, vol));
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (isPlaying) {
            RECT rect = { 380, 10, 460, 90 };
            double progress = (double)elapsedSeconds / (totalSeconds * 10); // 10 ticks per second
            int angle = (int)(progress * 360);

            HBRUSH blueBrush = CreateSolidBrush(RGB(0, 120, 215));
            HBRUSH emptyBrush = CreateSolidBrush(RGB(230, 230, 230));
            SelectObject(hdc, GetStockObject(NULL_PEN));

            // Background Circle
            SelectObject(hdc, emptyBrush);
            Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);

            // Progress Wedge
            SelectObject(hdc, blueBrush);
            int endX = rect.left + 40 + (int)(40 * sin(progress * 2 * PI));
            int endY = rect.top + 40 - (int)(40 * cos(progress * 2 * PI));
            Pie(hdc, rect.left, rect.top, rect.right, rect.bottom, rect.left + 40, rect.top, endX, endY);

            DeleteObject(blueBrush);
            DeleteObject(emptyBrush);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_TIMER:
        if (wParam == 1) StopAudio(hwnd, true);
        if (wParam == 2) { elapsedSeconds++; InvalidateRect(hwnd, NULL, FALSE); }
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == 99) StopAudio(hwnd, false);
        else PlayTone(hwnd, LOWORD(wParam) * 60);
        return 0;
    case WM_DESTROY:
        StopAudio(hwnd, false);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hI, HINSTANCE, PWSTR, int nC) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hI;
    wc.lpszClassName = L"NoNauseaTone";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"NoNausea 100Hz", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 500, 140, NULL, NULL, hI, NULL);
    ShowWindow(hwnd, nC);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    return 0;
}
