// Lumina Gear - Interactive Mechanical Power & Electrical Illumination Assembly.
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

const wchar_t* const kClassicPartNames[kPartCount] = {
    L"Motor", L"Small gear (12 teeth)", L"Large gear (18 teeth)", L"Pulley", L"Belt wheel", L"Lightbulb"};

const wchar_t* const kEnginePartNames[kSlotCount] = {
    L"12T Crankshaft Sprocket",
    L"24T Camshaft Sprocket",
    L"Timing Belt Tensioner",
    L"4-Stroke Cylinder & Valvetrain",
    L"Alternator Generator Unit",
    L"Workshop Illumination Bulb"};

struct App {
    Framebuffer canvas{kSceneWidth, kSceneHeight};
    Framebuffer fb{kSceneWidth, kSceneHeight};
    std::vector<std::uint32_t> screen = std::vector<std::uint32_t>(kSceneWidth * kSceneHeight);
    SceneView view;
    Simulation sim{SimulationMode::EngineValvetrain};
    bool trackingMouse = false;
    bool isDragging = false;
    int draggedPart = -1;
    float dragOffsetX = 0.0f;
    float dragOffsetY = 0.0f;
    LARGE_INTEGER perfFreq{};
    LARGE_INTEGER lastTime{};
};

App app;

int buttonAt(int x, int y) {
    if (hitButton(playButton(), x, y)) return 0;
    if (hitButton(pauseButton(), x, y)) return 1;
    if (hitButton(resetButton(), x, y)) return 2;
    if (hitButton(clearButton(), x, y)) return 3;
    if (hitButton(assembleButton(), x, y)) return 4;
    if (hitButton(modeButton(), x, y)) return 5;
    return -1;
}

bool onCanvas(int x, int y) { return x >= kToolbarWidth && y >= kTopBarHeight; }

void syncViewWithSimulation() {
    const bool isEngine = (app.sim.mode() == SimulationMode::EngineValvetrain);
    app.view.isEngineMode = isEngine;
    app.view.powered = app.sim.isPowered();
    app.view.powerLevel = app.sim.powerLevel();
    app.view.objective = app.sim.currentObjective();
    app.view.assemblyComplete = app.sim.isAssemblyComplete();

    if (isEngine) {
        app.view.crankAngle = app.sim.crankAngle();
        app.view.camAngle = app.sim.camAngle();
        app.view.intakeLift = app.sim.intakeValveLift();
        app.view.exhaustLift = app.sim.exhaustValveLift();
        app.view.pistonY = app.sim.pistonY();
        app.view.rodAngle = app.sim.connectingRodAngle();
        app.view.stroke = app.sim.strokeNumber();
        app.view.combustion = app.sim.isCombustion();
        app.view.beltTensioned = app.sim.isBeltTensioned();
        app.view.alternatorDriven = app.sim.isAlternatorDriven();
        app.view.wiringConnected = app.sim.isWiringConnected();
        app.view.collision = app.sim.isInterferenceCollision();
        app.view.rpm = app.sim.engineRpm();
        app.view.alternatorRpm = app.sim.alternatorRpm();
        app.view.voltage = app.sim.electricalVoltage();
        app.view.hp = app.sim.horsepower();
        app.view.isMeshed = app.sim.isBeltTensioned();
    } else {
        if (app.view.selectedPart >= 0) {
            app.view.isMeshed = app.sim.isPartMeshed(app.view.selectedPart);
        }
    }
}

void render(HWND hwnd, bool canvasChanged) {
    syncViewWithSimulation();

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

    std::wstring title;
    if (app.sim.mode() == SimulationMode::EngineValvetrain) {
        title = L"Lumina Gear - Engine Power Plant | ";
        if (app.sim.isAssemblyComplete()) {
            title += app.sim.isPlaying() ? L"RUNNING" : L"READY";
            title += L" | " + std::to_wstring(static_cast<int>(std::lround(app.sim.engineRpm()))) + L" RPM";
            title += L" | " + std::to_wstring(static_cast<int>(std::lround(app.sim.electricalVoltage()))) + L"V DC";
            const int pwr = static_cast<int>(std::lround(app.sim.powerLevel() * 100.0f));
            title += L" | Light: " + std::to_wstring(pwr) + L"%";
            if (app.sim.isInterferenceCollision()) title += L" [COLLISION CRASH!]";
        } else {
            title += L"ASSEMBLY MODE: Build the system from the toolbox!";
        }
    } else {
        title = L"Lumina Gear CP2 - ";
        title += app.sim.isPlaying() ? L"Running" : L"Stopped";
        const int powerPct = static_cast<int>(std::lround(app.sim.powerLevel() * 100.0f));
        title += L" | Power: " + std::to_wstring(powerPct) + L"%";
    }

    SetWindowTextW(hwnd, title.c_str());
    InvalidateRect(hwnd, nullptr, FALSE);
}

void setView(HWND hwnd, const SceneView& v) {
    const SceneView& o = app.view;
    const bool canvasChanged = v.powered != o.powered || v.selectedPart != o.selectedPart ||
                               v.isMeshed != o.isMeshed || v.powerLevel != o.powerLevel;
    app.view = v;
    render(hwnd, canvasChanged || v.hoveredSlot != o.hoveredSlot || v.hoveredButton != o.hoveredButton);
}

void onMouseMove(HWND hwnd, int x, int y) {
    app.view.mouseX = x;
    app.view.mouseY = y;

    if (app.isDragging && app.draggedPart >= 0) {
        float newX = x - app.dragOffsetX;
        float newY = y - app.dragOffsetY;
        const int r = app.sim.part(app.draggedPart).radius;
        newX = std::clamp(newX, kToolbarWidth + r + 10.0f, static_cast<float>(kSceneWidth - r - 10));
        newY = std::clamp(newY, kTopBarHeight + r + 10.0f, static_cast<float>(kSceneHeight - r - 10));

        app.sim.setPartPosition(app.draggedPart, newX, newY);
        render(hwnd, true);
        return;
    }

    if (!app.trackingMouse) {
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
        switch (btn) {
            case 0: // PLAY
                app.sim.play();
                break;
            case 1: // PAUSE
                app.sim.pause();
                break;
            case 2: // RESET
                app.sim.reset();
                app.view.selectedPart = -1;
                break;
            case 3: // CLEAR (Start assembly from scratch)
                app.sim.clearWorkbench();
                app.view.selectedPart = -1;
                break;
            case 4: // ASSEMBLE (Auto-build all components)
                app.sim.assembleAll();
                app.view.selectedPart = -1;
                break;
            case 5: // MODE TOGGLE
                if (app.sim.mode() == SimulationMode::EngineValvetrain) {
                    app.sim.setMode(SimulationMode::ClassicLumina);
                } else {
                    app.sim.setMode(SimulationMode::EngineValvetrain);
                }
                app.view.selectedPart = -1;
                break;
        }
        render(hwnd, true);
        return;
    }

    // Click on toolbar slot: pick up or spawn component onto canvas
    const int slot = toolbarSlotAt(x, y);
    if (slot >= 0 && slot < kSlotCount && app.sim.mode() == SimulationMode::EngineValvetrain) {
        app.isDragging = true;
        app.draggedPart = slot;
        app.view.selectedPart = slot;

        // Position part at mouse release point on canvas
        const float spawnX = std::max(static_cast<float>(kToolbarWidth + 60), static_cast<float>(x + 80));
        const float spawnY = static_cast<float>(y);
        app.sim.placePartAt(slot, spawnX, spawnY);
        app.dragOffsetX = x - app.sim.part(slot).x;
        app.dragOffsetY = y - app.sim.part(slot).y;
        SetCapture(hwnd);
        render(hwnd, true);
        return;
    }

    // Click on canvas: pick up placed component
    if (onCanvas(x, y)) {
        const int hit = scenePartAt(app.sim.parts(), x, y);
        if (hit >= 0) {
            app.isDragging = true;
            app.draggedPart = hit;
            app.view.selectedPart = hit;
            app.dragOffsetX = x - app.sim.part(hit).x;
            app.dragOffsetY = y - app.sim.part(hit).y;
            SetCapture(hwnd);
            render(hwnd, true);
        } else {
            app.view.selectedPart = -1;
            render(hwnd, true);
        }
    }
}

void onMouseUp(HWND hwnd, int, int) {
    if (app.isDragging) {
        ReleaseCapture();
        if (app.draggedPart >= 0) {
            app.sim.snapToMesh(app.draggedPart);
        }
        app.isDragging = false;
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

    if (wasMoving || isMoving || app.sim.isCombustion()) {
        render(hwnd, true);
    }
}

void paint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hwnd, &ps);
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = kSceneWidth;
    bmi.bmiHeader.biHeight = -kSceneHeight;
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
        case WM_KEYDOWN:
            switch (wp) {
                case VK_SPACE:
                    app.sim.togglePlay();
                    render(hwnd, true);
                    return 0;
                case 'R':
                    app.sim.reset();
                    render(hwnd, true);
                    return 0;
                case 'C':
                    app.sim.clearWorkbench();
                    render(hwnd, true);
                    return 0;
                case 'A':
                    app.sim.assembleAll();
                    render(hwnd, true);
                    return 0;
                case 'M':
                    if (app.sim.mode() == SimulationMode::EngineValvetrain) {
                        app.sim.setMode(SimulationMode::ClassicLumina);
                    } else {
                        app.sim.setMode(SimulationMode::EngineValvetrain);
                    }
                    render(hwnd, true);
                    return 0;
            }
            break;
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
                    buttonAt(p.x, p.y) >= 0 || toolbarSlotAt(p.x, p.y) >= 0 ||
                    (onCanvas(p.x, p.y) && scenePartAt(app.sim.parts(), p.x, p.y) >= 0);
                SetCursor(LoadCursor(nullptr, clickable ? IDC_HAND : IDC_ARROW));
                return TRUE;
            }
            break;
        case WM_ERASEBKGND: return 1;
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

    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT r{0, 0, kSceneWidth, kSceneHeight};
    AdjustWindowRect(&r, style, FALSE);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Lumina Gear - Engine Power Plant", style,
                              CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
                              nullptr, nullptr, instance, nullptr);
    if (!hwnd) return 1;

    QueryPerformanceFrequency(&app.perfFreq);
    QueryPerformanceCounter(&app.lastTime);
    SetTimer(hwnd, 1, 16, nullptr);

    render(hwnd, true);
    ShowWindow(hwnd, show);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
