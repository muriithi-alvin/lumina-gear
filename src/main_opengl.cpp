// main_opengl.cpp - Interactive OpenGL Window for Checkpoint 3 (CP3)
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>

#include "lumina/simulation.h"
#include "lumina/gl_renderer.h"
#include "lumina/scene.h"

namespace {

using namespace lumina;

struct AppGL {
    Simulation sim{SimulationMode::ClassicLumina};
    SceneView view;
    HDC hdc = nullptr;
    HGLRC hglrc = nullptr;
    LARGE_INTEGER perfFreq{};
    LARGE_INTEGER lastTime{};
};

AppGL app;

void syncView() {
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
        app.view.voltage = app.sim.electricalVoltage();
    } else {
        app.view.reversedPolarity = app.sim.isReversedPolarity();
    }
}

void initOpenGL(HWND hwnd) {
    app.hdc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    const int format = ChoosePixelFormat(app.hdc, &pfd);
    SetPixelFormat(app.hdc, format, &pfd);
    app.hglrc = wglCreateContext(app.hdc);
    wglMakeCurrent(app.hdc, app.hglrc);

    RECT rc;
    GetClientRect(hwnd, &rc);
    GLRenderer::init(rc.right - rc.left, rc.bottom - rc.top);
}

void cleanupOpenGL(HWND hwnd) {
    if (app.hglrc) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(app.hglrc);
        app.hglrc = nullptr;
    }
    if (app.hdc) {
        ReleaseDC(hwnd, app.hdc);
        app.hdc = nullptr;
    }
}

void renderGL(HWND hwnd) {
    syncView();
    RECT rc;
    GetClientRect(hwnd, &rc);
    GLRenderer::render(app.sim.parts(), app.view, rc.right - rc.left, rc.bottom - rc.top);
    SwapBuffers(app.hdc);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            initOpenGL(hwnd);
            QueryPerformanceFrequency(&app.perfFreq);
            QueryPerformanceCounter(&app.lastTime);
            SetTimer(hwnd, 1, 16, nullptr); // ~60 FPS
            return 0;

        case WM_SIZE:
            GLRenderer::reshape(LOWORD(lp), HIWORD(lp));
            renderGL(hwnd);
            return 0;

        case WM_TIMER: {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            const double dt = static_cast<double>(now.QuadPart - app.lastTime.QuadPart) / app.perfFreq.QuadPart;
            app.lastTime = now;
            app.sim.update(static_cast<float>(dt));
            renderGL(hwnd);
            return 0;
        }

        case WM_KEYDOWN:
            switch (wp) {
                case VK_SPACE:
                    app.sim.togglePlay();
                    return 0;
                case 'R':
                    app.sim.reset();
                    return 0;
                case 'C':
                    app.sim.clearWorkbench();
                    return 0;
                case 'S':
                    app.sim.solveLevel();
                    return 0;
                case 'B':
                    app.sim.toggleCrossedBelt();
                    return 0;
                case 'H':
                    app.view.tipCardMinimized = !app.view.tipCardMinimized;
                    return 0;
                case '1': case '2': case '3': case '4': case '5':
                    app.sim.setLevel(static_cast<int>(wp - '0'));
                    return 0;
            }
            break;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            cleanupOpenGL(hwnd);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

}  // namespace

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int show) {
    const wchar_t kClassName[] = L"LuminaGearGLWindow";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        0, kClassName,
        L"Lumina Gear - Checkpoint 3: OpenGL Hardware Accelerated Prototype",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1376, 799,
        nullptr, nullptr, inst, nullptr);

    if (!hwnd) return 1;

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
