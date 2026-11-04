// Lumina Gear - CP2: interactive window with continuous animation and drag-and-drop.
// Every pixel is still drawn by our own algorithms into a Framebuffer. Win32
// is only used to show the finished picture and to report the mouse.
//   Play: start continuous rotation & power flow
//   Pause: stop rotation (with inertia/coasting)
//   Reset: stop and reset machine positions & angles
//   Click & Drag: translate parts in 2D and reconfigure connections dynamically!
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "lumina/framebuffer.h"
#include "lumina/scene.h"
#include "lumina/simulation.h"
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
    Simulation sim;
    bool trackingMouse = false;
    bool isDragging = false;
    int draggedPart = -1;
    float dragOffsetX = 0.0f;
    float dragOffsetY = 0.0f;
    LARGE_INTEGER perfFreq{};
    LARGE_INTEGER lastTime{};
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
// layout that Windows expects.
void render(HWND hwnd, bool canvasChanged) {
    if (canvasChanged) {
        renderCanvas(app.canvas, app.sim.parts(), app.view);
    }
    app.fb = app.canvas;
    renderInterface(app.fb, app.view);
    for (int y = 0; y < kSceneHeight; ++y) {
        for (int x = 0; x < kSceneWidth; ++x) {
            const Color c = app.fb.getPixel(x, y);
            app.screen[static_cast<size_t>(y) * kSceneWidth + x] =
                (std::uint32_t{c.r} << 16) | (std::uint32_t{c.g} << 8) | std::uint32_t{c.b};
        }
    }

    std::wstring title = L"Lumina Gear CP2 - ";
    title += app.sim.isPlaying() ? L"Running" : (app.sim.motorSpeed() > 0.05f ? L"Coasting" : L"Stopped");
    const int powerPct = static_cast<int>(std::lround(app.sim.powerLevel() * 100.0f));
    title += L" | Power: " + std::to_wstring(powerPct) + L"%";

    if (app.view.selectedPart >= 0 && app.view.selectedPart < kPartCount) {
        title += L" | Selected: ";
        title += kPartNames[app.view.selectedPart];
        const float spd = app.sim.speed(app.view.selectedPart);
        wchar_t spdBuf[32];
        swprintf_s(spdBuf, L" (%.1f rad/s)", spd);
        title += spdBuf;
        if (app.view.isMeshed) {
            title += L" [Meshed]";
        }
    }
    SetWindowTextW(hwnd, title.c_str());
    InvalidateRect(hwnd, nullptr, FALSE);
}

// Redraw only when something visible has changed.
void setView(HWND hwnd, const SceneView& v) {
    const SceneView& o = app.view;
    const bool canvasChanged = v.powered != o.powered || v.selectedPart != o.selectedPart ||
                               v.isMeshed != o.isMeshed || v.powerLevel != o.powerLevel;
    if (!canvasChanged && v.hoveredButton == o.hoveredButton && v.hoveredSlot == o.hoveredSlot) return;
    app.view = v;
    render(hwnd, canvasChanged);
}

void onMouseMove(HWND hwnd, int x, int y) {
    if (app.isDragging && app.draggedPart >= 0) {
        // 2D Translation: translate part by mouse offset
        float newX = x - app.dragOffsetX;
        float newY = y - app.dragOffsetY;
        const int r = app.sim.part(app.draggedPart).radius;
        newX = std::clamp(newX, kToolbarWidth + r + 10.0f, static_cast<float>(kSceneWidth - r - 10));
        newY = std::clamp(newY, kTopBarHeight + r + 10.0f, static_cast<float>(kSceneHeight - r - 10));

        app.sim.setPartPosition(app.draggedPart, newX, newY);
        app.view.isMeshed = app.sim.isPartMeshed(app.draggedPart);
        app.view.powered = app.sim.isPowered();
        app.view.powerLevel = app.sim.powerLevel();
        render(hwnd, true);
        return;
    }

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

void onMouseDown(HWND hwnd, int x, int y) {
    const int btn = buttonAt(x, y);
    if (btn >= 0) {
        SceneView v = app.view;
        switch (btn) {
            case 0:
                app.sim.play();
                v.powered = true;
                break;
            case 1:
                app.sim.pause();
                break;
            case 2:
                app.sim.reset();
                v.powered = false;
                v.selectedPart = -1;
                v.isMeshed = false;
                break;
        }
        v.powerLevel = app.sim.powerLevel();
        setView(hwnd, v);
        return;
    }

    if (onCanvas(x, y)) {
        const int hit = scenePartAt(app.sim.parts(), x, y);
        if (hit >= 0) {
            app.isDragging = true;
            app.draggedPart = hit;
            app.view.selectedPart = hit;
            app.view.isMeshed = app.sim.isPartMeshed(hit);
            app.dragOffsetX = x - app.sim.part(hit).x;
            app.dragOffsetY = y - app.sim.part(hit).y;
            SetCapture(hwnd);
            render(hwnd, true);
        } else {
            app.view.selectedPart = -1;
            app.view.isMeshed = false;
            render(hwnd, true);
        }
    }
}

void onMouseUp(HWND hwnd, int, int) {
    if (app.isDragging) {
        ReleaseCapture();
        if (app.draggedPart >= 0) {
            // Snap to mesh if gear
            app.sim.snapToMesh(app.draggedPart);
            app.view.isMeshed = app.sim.isPartMeshed(app.draggedPart);
            app.view.powered = app.sim.isPowered();
            app.view.powerLevel = app.sim.powerLevel();
        }
        app.isDragging = false;
        app.draggedPart = -1;
        render(hwnd, true);
    }
}

void onTimer(HWND hwnd) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    const double dt = static_cast<double>(now.QuadPart - app.lastTime.QuadPart) / app.perfFreq.QuadPart;
    app.lastTime = now;

    const bool wasMoving = (app.sim.isPlaying() || std::abs(app.sim.motorSpeed()) > 0.01f);
    app.sim.update(static_cast<float>(dt));
    const bool isMoving = (app.sim.isPlaying() || std::abs(app.sim.motorSpeed()) > 0.01f);

    app.view.powered = app.sim.isPowered();
    app.view.powerLevel = app.sim.powerLevel();
    if (app.view.selectedPart >= 0) {
        app.view.isMeshed = app.sim.isPartMeshed(app.view.selectedPart);
    }

    if (wasMoving || isMoving) {
        render(hwnd, true);
    }
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
        case WM_TIMER: onTimer(hwnd); return 0;
        case WM_MOUSEMOVE: onMouseMove(hwnd, GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); return 0;
        case WM_MOUSELEAVE: {
            app.trackingMouse = false;
            SceneView v = app.view;
            v.hoveredButton = v.hoveredSlot = -1;
            setView(hwnd, v);
            return 0;
        }
        case WM_LBUTTONDOWN: onMouseDown(hwnd, GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); return 0;
        case WM_LBUTTONUP: onMouseUp(hwnd, GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); return 0;
        case WM_SETCURSOR:
            if (LOWORD(lp) == HTCLIENT) {
                POINT p;
                GetCursorPos(&p);
                ScreenToClient(hwnd, &p);
                if (app.isDragging) {
                    SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
                    return TRUE;
                }
                const bool clickable =
                    buttonAt(p.x, p.y) >= 0 || (onCanvas(p.x, p.y) && scenePartAt(app.sim.parts(), p.x, p.y) >= 0);
                SetCursor(LoadCursor(nullptr, clickable ? IDC_HAND : IDC_ARROW));
                return TRUE;
            }
            break;
        case WM_ERASEBKGND: return 1;   // WM_PAINT covers the whole window
        case WM_PAINT: paint(hwnd); return 0;
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
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
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Lumina Gear CP2", style, CW_USEDEFAULT, CW_USEDEFAULT,
                              r.right - r.left, r.bottom - r.top, nullptr, nullptr, instance, nullptr);
    if (!hwnd) return 1;

    QueryPerformanceFrequency(&app.perfFreq);
    QueryPerformanceCounter(&app.lastTime);
    SetTimer(hwnd, 1, 16, nullptr);   // ~60 FPS animation loop

    render(hwnd, true);
    ShowWindow(hwnd, show);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
