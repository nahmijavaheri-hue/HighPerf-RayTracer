//
// Created by Nahmi on 2026-07-23.
//

#ifndef HIGHPERF_RAYTRACER_WINDOW_H
#define HIGHPERF_RAYTRACER_WINDOW_H

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <functional>
#include <vector>

#include "main.h"

struct RenderSettings {
    int imageWidth;
    int imageHeight;
    int numSamplesPerPixel;
    int maxBounces;
};

class SettingsWindow {
public:
    SettingsWindow(int width, int height);
    ~SettingsWindow() = default;

    void show();
    HWND getHwnd() const { return hwnd; }

    RenderSettings getSettings() const;
    void setRenderButtonEnabled(bool enabled) { EnableWindow(renderButton, enabled); }

    // invoked (on the GUI thread, from WM_COMMAND) when the Render button is clicked
    std::function<void()> onRenderClicked;

private:
    static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static HWND createLabel(HWND parent, HINSTANCE inst, const wchar_t *text, int x, int y, int w, int h);
    static int getFieldInt(HWND control);
    // GetWindowText doesn't reliably return the selected item's text for
    // CBS_DROPDOWNLIST combos (no real edit-control portion backs it) — this
    // is the documented-correct way: get the selected index, then that index's text.
    static void getComboSelectedText(HWND combo, wchar_t *buffer, int bufferSize);

    HWND hwnd = nullptr;
    HWND resolutionCombo = nullptr;
    HWND widthEdit = nullptr;
    HWND heightEdit = nullptr;
    HWND aspectCombo = nullptr;
    HWND aaCombo = nullptr;
    HWND samplesCombo = nullptr;
    HWND boundsCombo = nullptr;
    HWND renderButton = nullptr;

    static constexpr int IDC_RESOLUTION = 101;
    static constexpr int IDC_WIDTH = 102;
    static constexpr int IDC_HEIGHT = 103;
    static constexpr int IDC_ASPECT = 104;
    static constexpr int IDC_AA = 105;
    static constexpr int IDC_SAMPLES = 106;
    static constexpr int IDC_BOUNDS = 107;
    static constexpr int IDC_RENDER = 108;
};

inline HWND SettingsWindow::createLabel(HWND parent, HINSTANCE inst, const wchar_t *text, int x, int y, int w, int h) {
    return CreateWindowEx(0, L"STATIC", text, WS_CHILD | WS_VISIBLE,
                           x, y, w, h, parent, nullptr, inst, nullptr);
}

inline int SettingsWindow::getFieldInt(HWND control) {
    wchar_t buffer[32];
    GetWindowText(control, buffer, 32);
    return _wtoi(buffer);
}

inline void SettingsWindow::getComboSelectedText(HWND combo, wchar_t *buffer, int bufferSize) {
    buffer[0] = L'\0';
    LRESULT index = SendMessage(combo, CB_GETCURSEL, 0, 0);
    if (index == CB_ERR) return;
    SendMessage(combo, CB_GETLBTEXT, static_cast<WPARAM>(index), reinterpret_cast<LPARAM>(buffer));
    (void) bufferSize; // CB_GETLBTEXT trusts the caller-provided buffer is big enough (matched via CB_GETLBTEXTLEN if needed)
}

inline SettingsWindow::SettingsWindow(int width, int height) {
    const wchar_t *className = L"RayTracerSettingsWindow";

    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProcStatic;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    wc.lpszClassName = className;
    RegisterClassEx(&wc);

    RECT clientRect = {0, 0, width, height};
    AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
    int windowWidth = clientRect.right - clientRect.left;
    int windowHeight = clientRect.bottom - clientRect.top;

    hwnd = CreateWindowEx(
        0, className, L"HighPerf RayTracer — Settings",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
        nullptr, nullptr, wc.hInstance,
        this
    );

    HINSTANCE inst = wc.hInstance;
    const int labelX = 10, controlX = 140, rowH = 32;
    int y = 10;

    createLabel(hwnd, inst, L"Resolution:", labelX, y, 120, 20);
    resolutionCombo = CreateWindowEx(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                      controlX, y, 160, 200, hwnd, (HMENU) (INT_PTR) IDC_RESOLUTION, inst, nullptr);
    const wchar_t *resolutions[] = {L"800 x 450", L"1600 x 900", L"1920 x 1080", L"3840 x 2160", L"Custom"};
    for (auto r: resolutions) SendMessage(resolutionCombo, CB_ADDSTRING, 0, (LPARAM) r);
    SendMessage(resolutionCombo, CB_SETCURSEL, 1, 0); // default: 1600 x 900
    y += rowH;

    createLabel(hwnd, inst, L"Width:", labelX, y, 60, 20);
    widthEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"1600", WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_DISABLED,
                                controlX, y, 60, 22, hwnd, (HMENU) (INT_PTR) IDC_WIDTH, inst, nullptr);
    createLabel(hwnd, inst, L"Height:", controlX + 70, y, 50, 20);
    heightEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"900", WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_DISABLED,
                                 controlX + 125, y, 60, 22, hwnd, (HMENU) (INT_PTR) IDC_HEIGHT, inst, nullptr);
    y += rowH;

    // aspect ratio is just a convenience that recomputes Height from the current
    // Width whenever it's changed — Width/Height stay the actual source of truth,
    // so a custom, non-matching height is still possible by editing it directly
    createLabel(hwnd, inst, L"Aspect Ratio:", labelX, y, 120, 20);
    aspectCombo = CreateWindowEx(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_DISABLED,
                                  controlX, y, 160, 200, hwnd, (HMENU) (INT_PTR) IDC_ASPECT, inst, nullptr);
    const wchar_t *aspects[] = {L"16:9", L"4:3", L"1:1", L"21:9"};
    for (auto a: aspects) SendMessage(aspectCombo, CB_ADDSTRING, 0, (LPARAM) a);
    SendMessage(aspectCombo, CB_SETCURSEL, 0, 0);
    y += rowH;

    createLabel(hwnd, inst, L"Anti-Aliasing:", labelX, y, 120, 20);
    aaCombo = CreateWindowEx(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                              controlX, y, 160, 200, hwnd, (HMENU) (INT_PTR) IDC_AA, inst, nullptr);
    const wchar_t *aaLevels[] = {L"Off", L"Low", L"Medium", L"High", L"Ultra"};
    for (auto a: aaLevels) SendMessage(aaCombo, CB_ADDSTRING, 0, (LPARAM) a);
    SendMessage(aaCombo, CB_SETCURSEL, 2, 0); // default: Medium
    y += rowH;

    // AA above is a preset that fills this field in; this field is what's actually
    // used at render time, and can still be hand-edited to any custom value
    createLabel(hwnd, inst, L"Samples/Pixel:", labelX, y, 120, 20);
    samplesCombo = CreateWindowEx(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_VSCROLL,
                                   controlX, y, 160, 200, hwnd, (HMENU) (INT_PTR) IDC_SAMPLES, inst, nullptr);
    const wchar_t *sampleOpts[] = {L"1", L"10", L"50", L"100", L"150", L"300", L"500"};
    for (auto s: sampleOpts) SendMessage(samplesCombo, CB_ADDSTRING, 0, (LPARAM) s);
    SetWindowText(samplesCombo, L"50");
    y += rowH;

    createLabel(hwnd, inst, L"Bounces:", labelX, y, 120, 20);
    boundsCombo = CreateWindowEx(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_VSCROLL,
                                  controlX, y, 160, 200, hwnd, (HMENU) (INT_PTR) IDC_BOUNDS, inst, nullptr);
    const wchar_t *boundsOpts[] = {L"5", L"10", L"20", L"50", L"100"};
    for (auto b: boundsOpts) SendMessage(boundsCombo, CB_ADDSTRING, 0, (LPARAM) b);
    SetWindowText(boundsCombo, L"10");
    y += rowH + 10;

    renderButton = CreateWindowEx(0, L"BUTTON", L"Render", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   controlX, y, 100, 30, hwnd, (HMENU) (INT_PTR) IDC_RENDER, inst, nullptr);
}

inline void SettingsWindow::show() {
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

inline RenderSettings SettingsWindow::getSettings() const {
    RenderSettings s{};

    wchar_t resText[32];
    getComboSelectedText(resolutionCombo, resText, 32);
    if (wcscmp(resText, L"Custom") == 0) {
        s.imageWidth = getFieldInt(widthEdit);
        s.imageHeight = getFieldInt(heightEdit);
    } else {
        swscanf(resText, L"%d x %d", &s.imageWidth, &s.imageHeight);
    }

    s.numSamplesPerPixel = getFieldInt(samplesCombo);
    s.maxBounces = getFieldInt(boundsCombo);

    if (s.imageWidth <= 0) s.imageWidth = 1600;
    if (s.imageHeight <= 0) s.imageHeight = 900;
    if (s.numSamplesPerPixel <= 0) s.numSamplesPerPixel = 50;
    if (s.maxBounces <= 0) s.maxBounces = 10;

    return s;
}

inline LRESULT CALLBACK SettingsWindow::WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SettingsWindow *self;

    if (msg == WM_NCCREATE) {
        auto *createStruct = reinterpret_cast<CREATESTRUCT *>(lParam);
        self = static_cast<SettingsWindow *>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<SettingsWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self) return self->handleMessage(hwnd, msg, wParam, lParam);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

inline LRESULT SettingsWindow::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_COMMAND: {
            const int id = LOWORD(wParam);
            const int notification = HIWORD(wParam);

            if (id == IDC_RESOLUTION && notification == CBN_SELCHANGE) {
                wchar_t sel[32];
                getComboSelectedText(resolutionCombo, sel, 32);
                BOOL custom = wcscmp(sel, L"Custom") == 0;
                EnableWindow(widthEdit, custom);
                EnableWindow(heightEdit, custom);
                EnableWindow(aspectCombo, custom);
                return 0;
            }

            if (id == IDC_ASPECT && notification == CBN_SELCHANGE) {
                wchar_t sel[32];
                getComboSelectedText(aspectCombo, sel, 32);
                int wRatio = 16, hRatio = 9;
                swscanf(sel, L"%d:%d", &wRatio, &hRatio);
                int w = getFieldInt(widthEdit);
                if (w <= 0) w = 1600;
                int h = static_cast<int>(w * static_cast<double>(hRatio) / wRatio);
                wchar_t buf[16];
                swprintf(buf, 16, L"%d", h);
                SetWindowText(heightEdit, buf);
                return 0;
            }

            if (id == IDC_AA && notification == CBN_SELCHANGE) {
                wchar_t sel[32];
                getComboSelectedText(aaCombo, sel, 32);
                const wchar_t *value = L"50";
                if (wcscmp(sel, L"Off") == 0) value = L"1";
                else if (wcscmp(sel, L"Low") == 0) value = L"10";
                else if (wcscmp(sel, L"Medium") == 0) value = L"50";
                else if (wcscmp(sel, L"High") == 0) value = L"150";
                else if (wcscmp(sel, L"Ultra") == 0) value = L"300";
                SetWindowText(samplesCombo, value);
                return 0;
            }

            if (id == IDC_RENDER && notification == BN_CLICKED) {
                if (onRenderClicked) onRenderClicked();
                return 0;
            }
            return 0;
        }

        // sent by the orchestrator thread once a render finishes, so the button
        // can't be clicked again mid-render (and re-clicked without a race once done)
        case WM_APP + 1:
            EnableWindow(renderButton, TRUE);
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
class PreviewWindow {
public:
    PreviewWindow(int width, int height);
    ~PreviewWindow();

    void show();
    HWND getHwnd() const { return hwnd; }

    void startLiveUpdates() {
        SetTimer(hwnd, TIMER_ID, 100, nullptr);
    }

    void attachRenderState(const std::vector<Color> *fb, const std::atomic<size_t> *completed,
                            size_t total, std::chrono::high_resolution_clock::time_point start) {
        framebuffer = fb;
        tilesCompleted = completed;
        totalTiles = total;
        startTime = start;
        hasRenderState = true;
    }

    // rebuilds the DIB and resizes the window for a new render at a different
    // resolution — used when Render is clicked again with different settings
    void reset(int newWidth, int newHeight) {
        if (dib) {
            DeleteObject(dib);
            dib = nullptr;
        }

        width = newWidth;
        height = newHeight;
        displayWidth = static_cast<int>(width * 0.6);
        displayHeight = static_cast<int>(height * 0.6);
        renderDone = false;

        RECT clientRect = {0, 0, displayWidth, displayHeight + barHeight};
        AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
        int windowWidth = clientRect.right - clientRect.left;
        int windowHeight = clientRect.bottom - clientRect.top;
        SetWindowPos(hwnd, nullptr, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);

        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = height;

        HDC hdc = GetDC(hwnd);
        dib = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pixelBits, nullptr, 0);
        ReleaseDC(hwnd, hdc);

        ZeroMemory(pixelBits, static_cast<size_t>(width) * height * 4);
        InvalidateRect(hwnd, nullptr, TRUE);
    }

private:
    // Win32 requires the window procedure to be a plain function pointer, so
    // WndProcStatic is what actually gets registered. It looks up which
    // PreviewWindow instance owns the HWND (stashed via GWLP_USERDATA during
    // WM_NCCREATE) and forwards to handleMessage() so the rest of this class
    // can use normal member variables instead of globals.
    static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    int width;
    int height;
    int displayWidth;
    int displayHeight;
    static constexpr int barHeight = 60;
    HWND hwnd = nullptr;
    HBITMAP dib = nullptr;
    void *pixelBits = nullptr;
    BITMAPINFO bmi{};

    static constexpr UINT_PTR TIMER_ID = 1;

    const std::vector<Color> *framebuffer = nullptr;
    const std::atomic<size_t> *tilesCompleted = nullptr;
    size_t totalTiles = 0;
    std::chrono::high_resolution_clock::time_point startTime;
    bool renderDone = false;
    bool hasRenderState = false; // false until attachRenderState() is called for the first time
};

inline PreviewWindow::PreviewWindow(int width, int height)
    : width(width), height(height),
      displayWidth(static_cast<int>(width * 0.6)), displayHeight(static_cast<int>(height * 0.6)) {
    const wchar_t *className = L"RayTracerPreviewWindow";

    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProcStatic;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszClassName = className;
    RegisterClassEx(&wc);

    // CreateWindowEx's size is the OUTER window (title bar + borders included),
    // not the drawable client area — AdjustWindowRect converts our desired
    // client size into the correct outer size, so the image + bar actually
    // fit without the bottom getting clipped by the window chrome.
    RECT clientRect = {0, 0, displayWidth, displayHeight + barHeight};
    AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
    int windowWidth = clientRect.right - clientRect.left;
    int windowHeight = clientRect.bottom - clientRect.top;

    // window is shown scaled down from the full render resolution — the
    // full-resolution image still gets rendered and saved, WM_PAINT just
    // scales it down when blitting via StretchBlt
    hwnd = CreateWindowEx(
        0, className, L"HighPerf RayTracer — Live Preview",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
        nullptr, nullptr, wc.hInstance,
        this // forwarded to WM_NCCREATE so WndProcStatic can find this instance
    );
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    // positive height = bottom-up DIB, which matches framebuffer's own convention:
    // framebuffer[0] is the BOTTOM of the camera image (v=0), not the top — a
    // negative (top-down) height here was what caused the upside-down image.
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(hwnd);
    dib = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pixelBits, nullptr, 0);
    ReleaseDC(hwnd, hdc);

    // blank until the first render actually starts
    ZeroMemory(pixelBits, static_cast<size_t>(width) * height * 4);
}

inline PreviewWindow::~PreviewWindow() {
   if (dib) DeleteObject(dib);
}

inline void PreviewWindow::show() {
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

inline LRESULT CALLBACK PreviewWindow::WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    PreviewWindow *self;

    if (msg == WM_NCCREATE) {
        auto *createStruct = reinterpret_cast<CREATESTRUCT *>(lParam);
        self = static_cast<PreviewWindow *>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<PreviewWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self) return self->handleMessage(hwnd, msg, wParam, lParam);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

inline LRESULT PreviewWindow::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdcPaint = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdcPaint);
            HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(memDC, dib));

            // HALFTONE gives a smoothed downscale instead of COLORONCOLOR's blocky
            // nearest-neighbor look; it requires resetting the brush origin right
            // after enabling it, or GDI can shift the output — a well-known quirk.
            SetStretchBltMode(hdcPaint, HALFTONE);
            SetBrushOrgEx(hdcPaint, 0, 0, nullptr);
            StretchBlt(hdcPaint, 0, 0, displayWidth, displayHeight,
                       memDC, 0, 0, width, height, SRCCOPY);
            SelectObject(memDC, oldBitmap);
            DeleteDC(memDC);

            // progress bar strip below the (scaled-down) image
            RECT barArea = {0, displayHeight, displayWidth, displayHeight + barHeight};
            FillRect(hdcPaint, &barArea, (HBRUSH) (COLOR_WINDOW + 1));

            double fraction = (tilesCompleted && totalTiles > 0)
                ? static_cast<double>(tilesCompleted->load()) / static_cast<double>(totalTiles)
                : 0.0;

            RECT border = {10, displayHeight + 8, displayWidth - 10, displayHeight + 28};
            RECT filled = border;
            filled.right = border.left + static_cast<LONG>(fraction * (border.right - border.left));

            HBRUSH greenBrush = CreateSolidBrush(RGB(80, 180, 80));
            FillRect(hdcPaint, &filled, greenBrush);
            DeleteObject(greenBrush);
            FrameRect(hdcPaint, &border, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

            size_t done = tilesCompleted ? tilesCompleted->load() : 0;

            wchar_t text[128];
            if (!hasRenderState) {
                // startTime hasn't been set yet — no render has ever run, so there's
                // nothing to compute elapsed time from
                swprintf(text, 128, L"Ready - pick your settings and click Render");
            } else {
                double elapsedSec = std::chrono::duration<double>(
                    std::chrono::high_resolution_clock::now() - startTime).count();
                if (renderDone) {
                    swprintf(text, 128, L"Done! %.1fs - saved to image.ppm", elapsedSec);
                } else {
                    swprintf(text, 128, L"%d%%   %zu / %zu tiles   %.1fs",
                             static_cast<int>(fraction * 100), done, totalTiles, elapsedSec);
                }
            }
            SetBkMode(hdcPaint, TRANSPARENT);
            TextOut(hdcPaint, 10, displayHeight + 34, text, static_cast<int>(wcslen(text)));

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_TIMER: {
            auto *bytes = static_cast<unsigned char *>(pixelBits);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    Color c = (*framebuffer)[y * width + x];
                    int i = (y * width + x) * 4;
                    bytes[i + 0] = static_cast<unsigned char>(255.99 * sqrt(std::clamp(c.z, 0.0f, 1.0f))); // B
                    bytes[i + 1] = static_cast<unsigned char>(255.99 * sqrt(std::clamp(c.y, 0.0f, 1.0f))); // G
                    bytes[i + 2] = static_cast<unsigned char>(255.99 * sqrt(std::clamp(c.x, 0.0f, 1.0f))); // R
                }
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_APP + 1:
            KillTimer(hwnd, TIMER_ID);
            renderDone = true;
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

#endif //HIGHPERF_RAYTRACER_WINDOW_H