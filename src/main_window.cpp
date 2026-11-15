// Lumina Gear - 5-Level Engineering Physics Campaign & Power Generation Game.
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

struct App {
    Framebuffer canvas{kSceneWidth, kSceneHeight};
    Framebuffer fb{kSceneWidth, kSceneHeight};
    std::vector<std::uint32_t> screen = std::vector<std::uint32_t>(kSceneWidth * kSceneHeight);
    SceneView view;
    Simulation sim{SimulationMode::ClassicLumina}; // Starts in Level 1 (Original Assignment)
    bool trackingMouse = false;
    bool isDragging = false;
    int draggedPart = -1;
    float dragOffsetX = 0.0f;
    float dragOffsetY = 0.0f;

    // Modular Tip Card interaction
    bool isDraggingCard = false;
    int cardDragOffsetX = 0;
    int cardDragOffsetY = 0;

    LARGE_INTEGER perfFreq{};
    LARGE_INTEGER lastTime{};
};

App app;

bool onCanvas(int x, int y) { return x >= kToolbarWidth && y >= kTopBarHeight; }

// Convert window client pixel coordinates to native framebuffer canvas coordinates (aspect-ratio aware)
void windowToCanvas(HWND hwnd, int clientX, int clientY, int& canvasX, int& canvasY) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    const int w = rc.right - rc.left;
    const int h = rc.bottom - rc.top;
    if (w > 0 && h > 0) {
        const float scale = std::min(static_cast<float>(w) / kSceneWidth,
                                     static_cast<float>(h) / kSceneHeight);
        const int destW = static_cast<int>(kSceneWidth * scale);
        const int destH = static_cast<int>(kSceneHeight * scale);
        const int destX = (w - destW) / 2;
        const int destY = (h - destH) / 2;

        if (scale > 0.0001f) {
            canvasX = std::clamp(static_cast<int>((clientX - destX) / scale), 0, kSceneWidth - 1);
            canvasY = std::clamp(static_cast<int>((clientY - destY) / scale), 0, kSceneHeight - 1);
        } else {
            canvasX = clientX;
            canvasY = clientY;
        }
    } else {
        canvasX = clientX;
        canvasY = clientY;
    }
}

void syncViewWithSimulation() {
    app.view.currentLevel = app.sim.currentLevel();
    app.view.levelCompleted = app.sim.isLevelCompleted();
    app.view.isEngineMode = (app.sim.mode() == SimulationMode::EngineValvetrain);
    app.view.powered = app.sim.isPowered();
    app.view.powerLevel = app.sim.powerLevel();
    app.view.objective = app.sim.currentObjective();
    app.view.failureReason = app.sim.failureReason();

    app.view.crossedBelt = app.sim.isCrossedBelt();
    if (app.view.isEngineMode) {
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
        app.view.reversedPolarity = app.sim.isReversedPolarity();
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

    std::wstring title = L"Lumina Gear - Level " + std::to_wstring(app.sim.currentLevel()) + L": ";
    switch (app.sim.currentLevel()) {
        case 1: title += L"The Lumina Machine (Assignment Baseline)"; break;
        case 2: title += L"Overdrive Speed Step-Up (Target 12V DC)"; break;
        case 3: title += L"Direction & Polarity Challenge"; break;
        case 4: title += L"4-Stroke Valvetrain Timing (Avoid Crash!)"; break;
        case 5: title += L"Master Engine-to-Electricity Power Plant"; break;
    }

    if (app.sim.isLevelCompleted()) {
        title += L" [GOAL SOLVED: 100%!]";
    } else if (app.sim.isInterferenceCollision()) {
        title += L" [CRASH: VALVE COLLISION!]";
    } else if (app.sim.isPlaying()) {
        title += L" [RUNNING]";
    } else {
        title += L" [PAUSED]";
    }

    SetWindowTextW(hwnd, title.c_str());
    InvalidateRect(hwnd, nullptr, FALSE);
}

void setView(HWND hwnd, const SceneView& v) {
    const SceneView& o = app.view;
    const bool canvasChanged = v.powered != o.powered || v.selectedPart != o.selectedPart ||
                               v.isMeshed != o.isMeshed || v.powerLevel != o.powerLevel ||
                               v.currentLevel != o.currentLevel;
    app.view = v;
    render(hwnd, canvasChanged || v.hoveredSlot != o.hoveredSlot || v.hoveredButton != o.hoveredButton);
}

void onMouseMove(HWND hwnd, int clientX, int clientY) {
    int x, y;
    windowToCanvas(hwnd, clientX, clientY, x, y);

    app.view.mouseX = x;
    app.view.mouseY = y;

    if (app.isDraggingCard) {
        int cardX, cardY, cardW, cardH;
        getTipCardBounds(app.sim.parts(), app.view, cardX, cardY, cardW, cardH);
        int newX = x - app.cardDragOffsetX;
        int newY = y - app.cardDragOffsetY;
        newX = std::clamp(newX, kToolbarWidth + 10, kSceneWidth - cardW - 10);
        newY = std::clamp(newY, kTopBarHeight + 10, kSceneHeight - cardH - 24);
        app.view.tipCardX = newX;
        app.view.tipCardY = newY;
        render(hwnd, true);
        return;
    }

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

static int slotToPartIndex(int slot, int currentLevel) {
    if (currentLevel <= 3) {
        switch (slot) {
            case 0: return kPartMotor;
            case 1: return kPartGear1;
            case 2: return kPartGear2;
            case 3: return kPartIdler;
            case 4: return kPartPulley;
            case 5: return kPartWheel;
            case 6: return kPartBulb;
            default: return -1;
        }
    } else {
        switch (slot) {
            case 0: return kEngineCrank;
            case 1: return kEngineCam;
            case 2: return kEngineCam; // 12T test sprocket
            case 3: return kEngineTensioner;
            case 4: return kEngineValvetrain;
            case 5: return kEngineAlternator;
            case 6: return kEngineBulb;
            default: return -1;
        }
    }
}

void onMouseDown(HWND hwnd, int clientX, int clientY) {
    int x, y;
    windowToCanvas(hwnd, clientX, clientY, x, y);

    // Modular Tip Card interaction (Drag or Toggle Collapse/Expand)
    int cardX, cardY, cardW, cardH;
    getTipCardBounds(app.sim.parts(), app.view, cardX, cardY, cardW, cardH);
    if (x >= cardX && x < cardX + cardW && y >= cardY && y < cardY + cardH) {
        if (app.view.tipCardMinimized || (x >= cardX + cardW - 65 && y < cardY + 20)) {
            app.view.tipCardMinimized = !app.view.tipCardMinimized;
            render(hwnd, true);
            return;
        }
        app.isDraggingCard = true;
        app.cardDragOffsetX = x - cardX;
        app.cardDragOffsetY = y - cardY;
        SetCapture(hwnd);
        return;
    }

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
            case 3: // CLEAR (Clear workbench for building)
                app.sim.clearWorkbench();
                app.view.selectedPart = -1;
                break;
            case 4: // SOLVE (Auto-assemble current level)
                app.sim.solveLevel();
                app.view.selectedPart = -1;
                break;
            case 5: // LVL 1
                app.sim.setLevel(1);
                app.view.selectedPart = -1;
                break;
            case 6: // LVL 2
                app.sim.setLevel(2);
                app.view.selectedPart = -1;
                break;
            case 7: // LVL 3
                app.sim.setLevel(3);
                app.view.selectedPart = -1;
                break;
            case 8: // LVL 4
                app.sim.setLevel(4);
                app.view.selectedPart = -1;
                break;
            case 9: // LVL 5
                app.sim.setLevel(5);
                app.view.selectedPart = -1;
                break;
        }
        render(hwnd, true);
        return;
    }

    // Click on toolbar slot: pick up component onto workbench
    const int slot = toolbarSlotAt(x, y);
    if (slot >= 0 && slot < kSlotCount) {
        const int partIdx = slotToPartIndex(slot, app.sim.currentLevel());
        if (partIdx >= 0 && partIdx < static_cast<int>(app.sim.parts().size())) {
            if (app.sim.currentLevel() >= 4 && (slot == 1 || slot == 2)) {
                // Instantly mount / swap sprocket directly on the camshaft at (320, 200)
                const int teeth = (slot == 2) ? 12 : 24;
                app.sim.part(kEngineCam).teeth = teeth;
                app.sim.part(kEngineCam).radius = static_cast<int>(gearPitchRadius(teeth));
                app.sim.part(kEngineCam).x = 320.0f;
                app.sim.part(kEngineCam).y = 200.0f;
                app.sim.part(kEngineCam).placed = true;
                app.sim.snapToMesh(kEngineCam);
                app.view.selectedPart = kEngineCam;
                render(hwnd, true);
                return;
            }

            app.isDragging = true;
            app.draggedPart = partIdx;
            app.view.selectedPart = partIdx;

            const float spawnX = std::max(static_cast<float>(kToolbarWidth + 60), static_cast<float>(x + 100));
            const float spawnY = static_cast<float>(y);
            app.sim.placePartAt(partIdx, spawnX, spawnY);
            app.dragOffsetX = x - app.sim.part(partIdx).x;
            app.dragOffsetY = y - app.sim.part(partIdx).y;
            SetCapture(hwnd);
            render(hwnd, true);
            return;
        }
    }

    // Click on canvas: drag existing placed component
    if (onCanvas(x, y)) {
        // Click near belt midpoint in classic mode: toggle Crossed Belt!
        if (app.sim.currentLevel() <= 3 && app.sim.part(kPartPulley).placed && app.sim.part(kPartWheel).placed) {
            const float midX = (app.sim.part(kPartPulley).x + app.sim.part(kPartWheel).x) * 0.5f;
            const float midY = (app.sim.part(kPartPulley).y + app.sim.part(kPartWheel).y) * 0.5f;
            if (std::hypot(static_cast<float>(x) - midX, static_cast<float>(y) - midY) <= 35.0f) {
                app.sim.toggleCrossedBelt();
                render(hwnd, true);
                return;
            }
        }

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

void onMouseUp(HWND hwnd, int clientX, int clientY) {
    if (app.isDraggingCard) {
        ReleaseCapture();
        app.isDraggingCard = false;
        render(hwnd, true);
        return;
    }
    if (app.isDragging) {
        ReleaseCapture();
        int x, y;
        windowToCanvas(hwnd, clientX, clientY, x, y);
        if (app.draggedPart >= 0) {
            if (x < kToolbarWidth + 20) {
                // Dragged back to toolbox -> Remove/delete component from canvas!
                app.sim.removePart(app.draggedPart);
                app.view.selectedPart = -1;
            } else {
                app.sim.snapToMesh(app.draggedPart);
            }
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
    RECT rc;
    GetClientRect(hwnd, &rc);
    const int w = rc.right - rc.left;
    const int h = rc.bottom - rc.top;

    const float scale = std::min(static_cast<float>(w) / kSceneWidth,
                                 static_cast<float>(h) / kSceneHeight);
    const int destW = static_cast<int>(kSceneWidth * scale);
    const int destH = static_cast<int>(kSceneHeight * scale);
    const int destX = (w - destW) / 2;
    const int destY = (h - destH) / 2;

    // Fill letterbox / pillarbox margins with app background RGB(18, 24, 29)
    if (destX > 0 || destY > 0) {
        HBRUSH bgBrush = CreateSolidBrush(RGB(18, 24, 29));
        if (destX > 0) {
            RECT leftR = {0, 0, destX, h};
            RECT rightR = {destX + destW, 0, w, h};
            FillRect(dc, &leftR, bgBrush);
            FillRect(dc, &rightR, bgBrush);
        }
        if (destY > 0) {
            RECT topR = {0, 0, w, destY};
            RECT bottomR = {0, destY + destH, w, h};
            FillRect(dc, &topR, bgBrush);
            FillRect(dc, &bottomR, bgBrush);
        }
        DeleteObject(bgBrush);
    }

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = kSceneWidth;
    bmi.bmiHeader.biHeight = -kSceneHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    SetStretchBltMode(dc, HALFTONE);
    SetBrushOrgEx(dc, 0, 0, nullptr);
    StretchDIBits(dc, destX, destY, destW, destH, 0, 0, kSceneWidth, kSceneHeight,
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
        case WM_RBUTTONDOWN: {
            int x, y;
            windowToCanvas(hwnd, GET_X_LPARAM(lp), GET_Y_LPARAM(lp), x, y);
            const int hit = scenePartAt(app.sim.parts(), x, y);
            if (hit >= 0) {
                app.sim.removePart(hit);
                if (app.view.selectedPart == hit) app.view.selectedPart = -1;
                render(hwnd, true);
            }
            return 0;
        }
        case WM_KEYDOWN:
            switch (wp) {
                case VK_DELETE:
                case VK_BACK:
                    if (app.view.selectedPart >= 0) {
                        app.sim.removePart(app.view.selectedPart);
                        app.view.selectedPart = -1;
                        render(hwnd, true);
                    }
                    return 0;
                case 'B':
                    app.sim.toggleCrossedBelt();
                    render(hwnd, true);
                    return 0;
                case 'H':
                    app.view.tipCardMinimized = !app.view.tipCardMinimized;
                    render(hwnd, true);
                    return 0;
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
                case 'S':
                    app.sim.solveLevel();
                    render(hwnd, true);
                    return 0;
                case '1': case '2': case '3': case '4': case '5':
                    app.sim.setLevel(static_cast<int>(wp - '0'));
                    render(hwnd, true);
                    return 0;
            }
            break;
        case WM_SETCURSOR:
            if (LOWORD(lp) == HTCLIENT) {
                POINT p;
                GetCursorPos(&p);
                ScreenToClient(hwnd, &p);
                int cx, cy;
                windowToCanvas(hwnd, p.x, p.y, cx, cy);
                if (app.isDragging || app.isDraggingCard) {
                    SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
                    return TRUE;
                }
                int cardX, cardY, cardW, cardH;
                getTipCardBounds(app.sim.parts(), app.view, cardX, cardY, cardW, cardH);
                const bool overCard = (cx >= cardX && cx < cardX + cardW && cy >= cardY && cy < cardY + cardH);
                const bool clickable =
                    overCard || buttonAt(cx, cy) >= 0 || toolbarSlotAt(cx, cy) >= 0 ||
                    (onCanvas(cx, cy) && scenePartAt(app.sim.parts(), cx, cy) >= 0);
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

    // Resizable window with minimize, maximize, and restore buttons!
    const DWORD style = WS_OVERLAPPEDWINDOW;
    RECT r{0, 0, kSceneWidth, kSceneHeight};
    AdjustWindowRect(&r, style, FALSE);

    // Center window on user's screen
    const int screenW = GetSystemMetrics(SM_CXSCREEN);
    const int screenH = GetSystemMetrics(SM_CYSCREEN);
    const int winW = r.right - r.left;
    const int winH = r.bottom - r.top;
    const int posX = std::max(0, (screenW - winW) / 2);
    const int posY = std::max(0, (screenH - winH) / 2);

    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Lumina Gear - Physics Campaign", style,
                              posX, posY, winW, winH, nullptr, nullptr, instance, nullptr);
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
