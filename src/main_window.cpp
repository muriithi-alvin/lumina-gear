// Lumina Gear - CP1: interactive window (Windows only).
// Every pixel is still drawn by our own algorithms into a Framebuffer. Win32
// is only used to show the finished picture and to report the mouse.
//   Play: power on    Pause: power off    Reset: power off and clear the selection
//   Click a part to select it (Point-in-Circle picking).
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <cstdint>
#include <string>
#include <vector>

#include "lumina/framebuffer.h"
#include "lumina/scene.h"
#include "lumina/ui.h"

namespace {

using namespace lumina;

const wchar_t* const kPartNames[kPartCount] = {
    L"Motor", L"Small gear (12 teeth)", L"Large gear (18 teeth)", L"Pulley", L"Belt wheel", L"Lightbulb"};

struct App {
    Framebuffer canvas{kSceneWidth, kSceneHeight};   // the scene without the interface
    Framebuffer fb{kSceneWidth, kSceneHeight};
    std::vector<std::uint32_t> screen = std::vector<std::uint32_t>(kSceneWidth * kSceneHeight);
    SceneView view;
    bool trackingMouse = false;
};

App app;   // one window, so one copy of the state

int buttonAt(int x, int y) {
    if (hitButton(playButton(), x, y)) return 0;
    if (hitButton(pauseButton(), x, y)) return 1;
    if (hitButton(resetButton(), x, y)) return 2;
    return -1;
}

bool onCanvas(int x, int y) { return x >= kToolbarWidth && y >= kTopBarHeight; }

// Draw the scene with our own code, then copy it into the 32-bit 0x00RRGGBB
// layout that Windows expects. The slow canvas is redrawn only when needed.
void render(HWND hwnd, bool canvasChanged) {
    if (canvasChanged) renderCanvas(app.canvas, app.view);
    app.fb = app.canvas;
    renderInterface(app.fb, app.view);
    for (int y = 0; y < kSceneHeight; ++y)
        for (int x = 0; x < kSceneWidth; ++x) {
            const Color c = app.fb.getPixel(x, y);
            app.screen[static_cast<size_t>(y) * kSceneWidth + x] =
                (std::uint32_t{c.r} << 16) | (std::uint32_t{c.g} << 8) | std::uint32_t{c.b};
        }

    std::wstring title = L"Lumina Gear CP1 - ";
    title += app.view.powered ? L"Powered" : L"Unpowered";
    if (app.view.selectedPart >= 0) {
        title += L" - Selected: ";
        title += kPartNames[app.view.selectedPart];
    }
    SetWindowTextW(hwnd, title.c_str());
    InvalidateRect(hwnd, nullptr, FALSE);
}

// Redraw only when something visible has changed.
void setView(HWND hwnd, const SceneView& v) {
    const SceneView& o = app.view;
    const bool canvasChanged = v.powered != o.powered || v.selectedPart != o.selectedPart;
    if (!canvasChanged && v.hoveredButton == o.hoveredButton && v.hoveredSlot == o.hoveredSlot) return;
    app.view = v;
    render(hwnd, canvasChanged);
}

void onMouseMove(HWND hwnd, int x, int y) {
    if (!app.trackingMouse) {   // ask for WM_MOUSELEAVE, so hovering ends when the mouse leaves
        TRACKMOUSEEVENT tme{};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd;
        app.trackingMouse = TrackMouseEvent(&tme) != FALSE;
    }
    SceneView v = app.view;
    v.hoveredButton = buttonAt(x, y);
    v.hoveredSlot = toolbarSlotAt(x, y);
    setView(hwnd, v);
}

void onClick(HWND hwnd, int x, int y) {
    SceneView v = app.view;
    switch (buttonAt(x, y)) {
        case 0: v.powered = true; break;
        case 1: v.powered = false; break;
        case 2: v.powered = false; v.selectedPart = -1; break;
        default:
            if (onCanvas(x, y)) v.selectedPart = scenePartAt(x, y);   // empty canvas clears it
    }
    setView(hwnd, v);
}

void paint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hwnd, &ps);
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = kSceneWidth;
    bmi.bmiHeader.biHeight = -kSceneHeight;   // negative: rows run top to bottom, like our framebuffer
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    StretchDIBits(dc, 0, 0, kSceneWidth, kSceneHeight, 0, 0, kSceneWidth, kSceneHeight,
                  app.screen.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_MOUSEMOVE: onMouseMove(hwnd, GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); return 0;
        case WM_MOUSELEAVE: {
            app.trackingMouse = false;
            SceneView v = app.view;
            v.hoveredButton = v.hoveredSlot = -1;
            setView(hwnd, v);
            return 0;
        }
        case WM_LBUTTONDOWN: onClick(hwnd, GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); return 0;
        case WM_SETCURSOR:
            if (LOWORD(lp) == HTCLIENT) {   // hand cursor over anything clickable
                POINT p;
                GetCursorPos(&p);
                ScreenToClient(hwnd, &p);
                const bool clickable =
                    buttonAt(p.x, p.y) >= 0 || (onCanvas(p.x, p.y) && scenePartAt(p.x, p.y) >= 0);
                SetCursor(LoadCursor(nullptr, clickable ? IDC_HAND : IDC_ARROW));
                return TRUE;
            }
            break;
        case WM_ERASEBKGND: return 1;   // WM_PAINT covers the whole window
        case WM_PAINT: paint(hwnd); return 0;
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"LuminaGearWindow";
    if (!RegisterClassW(&wc)) return 1;

    // Fixed size, so one window pixel is one framebuffer pixel.
    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT r{0, 0, kSceneWidth, kSceneHeight};
    AdjustWindowRect(&r, style, FALSE);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Lumina Gear CP1", style, CW_USEDEFAULT, CW_USEDEFAULT,
                              r.right - r.left, r.bottom - r.top, nullptr, nullptr, instance, nullptr);
    if (!hwnd) return 1;

    render(hwnd, true);
    ShowWindow(hwnd, show);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
