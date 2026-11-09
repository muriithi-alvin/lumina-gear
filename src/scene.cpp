#include "lumina/scene.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

#include "lumina/belt.h"
#include "lumina/circle.h"
#include "lumina/components.h"
#include "lumina/effects.h"
#include "lumina/line.h"
#include "lumina/picking.h"
#include "lumina/polygon.h"
#include "lumina/simulation.h"
#include "lumina/ui.h"

namespace lumina {

namespace {
constexpr float kPi = 3.14159265358979f;
int ri(float v) { return static_cast<int>(std::lround(v)); }

void drawGrid(Framebuffer& fb) {
    const Color dot(58, 74, 83);
    for (int y = kTopBarHeight + 20; y < fb.height(); y += 40)
        for (int x = kToolbarWidth + 20; x < fb.width(); x += 40) fb.setPixel(x, y, dot);
}

void drawShaft(Framebuffer& fb, const Component& a, const Component& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dist = std::hypot(dx, dy);
    if (dist > 300.0f) return;

    drawThickLine(fb, ri(a.x), ri(a.y), ri(b.x), ri(b.y), 8, palette::metalDark);
    drawThickLine(fb, ri(a.x), ri(a.y) - 2, ri(b.x), ri(b.y) - 2, 2, palette::metalLight);
}

struct PickCircle { int x, y, r; };

PickCircle pickCircle(const Component& c) {
    if (c.type == ComponentType::Motor) return {ri(c.x) - c.radius, ri(c.y), c.radius};
    return {ri(c.x), ri(c.y), c.radius};
}

// Renders the engine block castings, manifolds, and cylinder head
void drawEngineBlock(Framebuffer& fb, const SceneView& view) {
    // Engine block casting base behind cylinder
    const int bLeft = 530, bRight = 750;
    const int bTop = 230, bBot = 520;
    fillRect(fb, bLeft, bTop, bRight - bLeft, bBot - bTop, Color(38, 48, 56));
    drawRect(fb, bLeft, bTop, bRight - bLeft, bBot - bTop, palette::outline);

    // Cylinder head cover (top of valvetrain)
    fillRect(fb, 520, 110, 240, 70, Color(48, 60, 70));
    drawRect(fb, 520, 110, 240, 70, palette::outline);

    // Cylinder head bolts
    for (int bx : {540, 600, 680, 740}) {
        fillCircle(fb, bx, 125, 4, palette::metalLight);
        drawCircle(fb, bx, 125, 4, palette::outline);
    }

    // Intake Manifold runner (curves into intake port on left)
    const Color intakeCol = (view.stroke == 1 && view.rpm > 10.0f) ? palette::intakeAir : Color(60, 80, 95);
    drawThickLine(fb, 470, 200, 560, 245, 14, intakeCol);
    drawThickLine(fb, 470, 200, 560, 245, 2, palette::outline);

    // Flowing intake charge particles when intake stroke
    if (view.stroke == 1 && view.rpm > 10.0f) {
        drawText(fb, 450, 175, "AIR/FUEL IN", 1, palette::intakeAir);
        for (int p = 0; p < 4; ++p) {
            fillCircle(fb, 485 + p * 20, 208 + p * 10, 3, Color(160, 220, 255));
        }
    }

    // Exhaust Manifold runner (curves out from exhaust port on right)
    const Color exhaustCol = (view.stroke == 4 && view.rpm > 10.0f) ? palette::exhaustGas : Color(70, 55, 50);
    drawThickLine(fb, 720, 245, 810, 200, 14, exhaustCol);
    drawThickLine(fb, 720, 245, 810, 200, 2, palette::outline);

    // Flowing hot exhaust gas when exhaust stroke
    if (view.stroke == 4 && view.rpm > 10.0f) {
        drawText(fb, 760, 175, "EXHAUST OUT", 1, palette::exhaustGas);
        for (int p = 0; p < 4; ++p) {
            fillCircle(fb, 735 + p * 20, 237 - p * 10, 4, Color(255, 140, 40));
        }
    }

    // Spark Plug assembly at (640, 225)
    fillRect(fb, 636, 185, 8, 30, Color(240, 240, 245)); // ceramic insulator
    fillRect(fb, 634, 215, 12, 12, palette::metalLight);    // threaded steel hex body
    drawRect(fb, 634, 185, 12, 42, palette::outline);
    drawLine(fb, 640, 227, 640, 244, palette::metalDark);  // center electrode

    // Spark discharge during power stroke ignition
    if (view.combustion) {
        drawThickLine(fb, 638, 243, 642, 243, 3, Color(255, 255, 255));
        drawGlow(fb, 640, 243, 18, Color(100, 200, 255), 1.0f);
    }
}

// Renders the toothed rubber timing belt connecting Crank, Tensioner, and Cam sprockets
void drawTimingBelt(Framebuffer& fb, const Component& crank, const Component& cam, const Component& tensioner, bool tensioned) {
    const Color beltBase = tensioned ? palette::belt : Color(90, 75, 65);
    const Color toothCol = Color(40, 35, 30);

    // 1. Left straight run (Cam sprocket down to Crank sprocket)
    const float camLeftX = cam.x - cam.radius + 3;
    const float crankLeftX = crank.x - crank.radius + 3;
    drawThickLine(fb, ri(camLeftX), ri(cam.y), ri(crankLeftX), ri(crank.y), 8, beltBase);
    drawLine(fb, ri(camLeftX) - 4, ri(cam.y), ri(crankLeftX) - 4, ri(crank.y), palette::outline);
    drawLine(fb, ri(camLeftX) + 4, ri(cam.y), ri(crankLeftX) + 4, ri(crank.y), palette::outline);

    // Toothed notches on left run
    for (int y = ri(cam.y); y <= ri(crank.y); y += 12) {
        const float t = static_cast<float>(y - cam.y) / (crank.y - cam.y);
        const int x = ri(camLeftX + t * (crankLeftX - camLeftX));
        drawLine(fb, x - 2, y, x + 2, y, toothCol);
    }

    // 2. Bottom run from Crank sprocket to Tensioner pulley
    const float crankRightX = crank.x + crank.radius * 0.7f;
    const float crankRightY = crank.y + crank.radius * 0.7f;
    drawThickLine(fb, ri(crankRightX), ri(crankRightY), ri(tensioner.x), ri(tensioner.y + tensioner.radius), 8, beltBase);
    drawLine(fb, ri(crankRightX), ri(crankRightY) + 4, ri(tensioner.x), ri(tensioner.y + tensioner.radius) + 4, palette::outline);

    // 3. Upward run from Tensioner pulley to Cam sprocket
    const float camRightX = cam.x + cam.radius * 0.85f;
    const float camRightY = cam.y + cam.radius * 0.5f;
    drawThickLine(fb, ri(tensioner.x), ri(tensioner.y - tensioner.radius), ri(camRightX), ri(camRightY), 8, beltBase);
    drawLine(fb, ri(tensioner.x) + 4, ri(tensioner.y - tensioner.radius), ri(camRightX) + 4, ri(camRightY), palette::outline);

    // Tension status banner near tensioner
    if (tensioned) {
        drawText(fb, ri(tensioner.x) + 30, ri(tensioner.y) - 6, "TENSION: OK", 1, Color(100, 240, 140));
    } else {
        drawText(fb, ri(tensioner.x) + 30, ri(tensioner.y) - 6, "BELT SLACK", 1, Color(255, 90, 80));
    }

    // Timing pointer marks on engine casing above sprockets (TDC alignment notches)
    // Crank timing mark (12 o'clock)
    drawLine(fb, ri(crank.x), ri(crank.y - crank.radius - 8), ri(crank.x), ri(crank.y - crank.radius), Color(255, 220, 60));
    // Cam timing mark (12 o'clock)
    drawLine(fb, ri(cam.x), ri(cam.y - cam.radius - 8), ri(cam.x), ri(cam.y - cam.radius), Color(255, 220, 60));
}

// Renders the live digital dashboard / ECU readout
void drawEngineDashboard(Framebuffer& fb, const SceneView& view) {
    const int x0 = 760, y0 = 60, w = 220, h = 175;

    // Bezel & background
    fillRect(fb, x0, y0, w, h, Color(24, 32, 38));
    drawRect(fb, x0, y0, w, h, Color(60, 80, 95));
    drawRect(fb, x0 + 1, y0 + 1, w - 2, h - 2, palette::outline);

    // Title
    drawText(fb, x0 + 12, y0 + 10, "ECU DIAGNOSTICS", 1, palette::glow);
    drawLine(fb, x0 + 10, y0 + 24, x0 + w - 10, y0 + 24, Color(50, 65, 75));

    // 1. RPM
    char buf[32];
    std::snprintf(buf, sizeof(buf), "RPM: %4d", ri(view.rpm));
    drawText(fb, x0 + 12, y0 + 32, buf, 1, Color(230, 240, 245));

    // Tachometer bar
    const int barW = w - 24;
    const int rpmFill = std::clamp(ri(barW * (view.rpm / 2000.0f)), 0, barW);
    fillRect(fb, x0 + 12, y0 + 46, barW, 6, Color(40, 50, 60));
    fillRect(fb, x0 + 12, y0 + 46, rpmFill, 6, view.rpm > 1600 ? Color(255, 90, 80) : Color(100, 220, 140));
    drawRect(fb, x0 + 12, y0 + 46, barW, 6, palette::outline);

    // 2. Active 4-Stroke Cycle Stage
    const char* sName = "IDLE";
    Color sColor = Color(180, 190, 200);
    switch (view.stroke) {
        case 1: sName = "STROKE: INTAKE"; sColor = palette::intakeAir; break;
        case 2: sName = "STROKE: COMPRESSION"; sColor = Color(120, 200, 240); break;
        case 3: sName = "STROKE: POWER (FIRE)"; sColor = palette::combustionFire; break;
        case 4: sName = "STROKE: EXHAUST"; sColor = palette::exhaustGas; break;
    }
    drawText(fb, x0 + 12, y0 + 60, sName, 1, sColor);

    // 3. Synchronization & Ratio
    if (view.collision) {
        drawText(fb, x0 + 12, y0 + 80, "[JAMMED: COLLISION]", 1, Color(255, 60, 60));
    } else if (view.beltTensioned) {
        drawText(fb, x0 + 12, y0 + 80, "RATIO: 2:1 SYNC [OK]", 1, Color(100, 240, 140));
    } else {
        drawText(fb, x0 + 12, y0 + 80, "BELT SLIPPED [OUT OF SYNC]", 1, Color(255, 140, 50));
    }

    // 4. Valve lifts
    std::snprintf(buf, sizeof(buf), "IN LIFT:  %2.1f MM", view.intakeLift * 0.75f);
    drawText(fb, x0 + 12, y0 + 100, buf, 1, Color(190, 200, 210));
    std::snprintf(buf, sizeof(buf), "EX LIFT:  %2.1f MM", view.exhaustLift * 0.75f);
    drawText(fb, x0 + 12, y0 + 116, buf, 1, Color(190, 200, 210));

    // 5. Output Horsepower
    std::snprintf(buf, sizeof(buf), "OUTPUT: %3d HP", ri(view.hp));
    drawText(fb, x0 + 12, y0 + 138, buf, 1, palette::glow);

    // 6. Cam gear tooth count badge
    drawText(fb, x0 + 12, y0 + 154, "CRANK: 12T | CAM: 24T", 1, Color(140, 160, 175));
}

}  // namespace

std::vector<Component> sceneParts(bool powered) {
    const float midY = 330;
    std::vector<Component> parts(kPartCount);
    parts[kPartMotor] = makeMotor(210, midY, 46);
    parts[kPartGear1] = makeGear(340, midY, 12, 0);
    const float gear2X = parts[kPartGear1].x + gearPitchRadius(12) + gearPitchRadius(18);
    parts[kPartGear2] = makeGear(gear2X, midY, 18, meshAngle(18, kPi));
    parts[kPartPulley] = makePulley(gear2X + 160, midY, 40);
    parts[kPartWheel] = makePulley(820, 440, 22);
    parts[kPartBulb] = makeBulb(820, 270, 44, powered);
    return parts;
}

int scenePartAt(const std::vector<Component>& parts, int px, int py) {
    for (int i = static_cast<int>(parts.size()) - 1; i >= 0; --i) {
        const PickCircle pc = pickCircle(parts[i]);
        if (pointInCircle(px, py, pc.x, pc.y, pc.r)) return i;
    }
    return -1;
}

int scenePartAt(int px, int py) {
    const std::vector<Component> parts = sceneParts(false);
    return scenePartAt(parts, px, py);
}

void renderScene(Framebuffer& fb, bool powered) {
    SceneView view;
    view.powered = powered;
    renderScene(fb, view);
}

void renderScene(Framebuffer& fb, const SceneView& view) {
    renderCanvas(fb, view);
    renderInterface(fb, view);
}

void renderScene(Framebuffer& fb, const std::vector<Component>& parts, const SceneView& view) {
    renderCanvas(fb, parts, view);
    renderInterface(fb, view);
}

void renderCanvas(Framebuffer& fb, const SceneView& view) {
    const std::vector<Component> parts = sceneParts(view.powered);
    renderCanvas(fb, parts, view);
}

void renderCanvas(Framebuffer& fb, const std::vector<Component>& parts, const SceneView& view) {
    fb.clear(palette::background);
    drawGrid(fb);

    // =========================================================================
    // ENGINE VALVETRAIN ASSEMBLY RENDERING
    // =========================================================================
    if (view.isEngineMode) {
        if (parts.size() < kEnginePartCount) return;

        const Component& crank = parts[kEngineCrank];
        const Component& cam = parts[kEngineCam];
        const Component& tens = parts[kEngineTensioner];
        const Component& inCam = parts[kEngineIntakeCam];
        const Component& exCam = parts[kEngineExhaustCam];
        const Component& inValve = parts[kEngineIntakeValve];
        const Component& exValve = parts[kEngineExhaustValve];
        const Component& piston = parts[kEnginePiston];

        // 1. Engine Block casting & manifolds
        drawEngineBlock(fb, view);

        // 2. Timing Belt & Spanning Runs
        drawTimingBelt(fb, crank, cam, tens, view.beltTensioned);

        // 3. Overhead Camshaft (solid polished steel drive axle)
        drawThickLine(fb, ri(cam.x), ri(cam.y), 745, ri(cam.y), 10, palette::metalMid);
        drawThickLine(fb, ri(cam.x), ri(cam.y) - 2, 745, ri(cam.y) - 2, 3, palette::metalLight);
        // Camshaft bearing journal pedestals
        fillRect(fb, 410, ri(cam.y) - 10, 24, 20, palette::metalDark);
        drawRect(fb, 410, ri(cam.y) - 10, 24, 20, palette::outline);
        fillRect(fb, 735, ri(cam.y) - 10, 20, 20, palette::metalDark);
        drawRect(fb, 735, ri(cam.y) - 10, 20, 20, palette::outline);

        // 4. Cylinder Bore, Piston, Connecting Rod & Combustion Flames
        drawPistonAndCylinder(fb, piston.x, piston.y, 140.0f, 190.0f,
                              view.pistonY, view.rodAngle, view.combustion);

        // 5. Overhead Cam Lobes (rotate at 0.5x crank speed)
        drawCamLobe(fb, inCam.x, inCam.y, static_cast<float>(inCam.radius), inCam.angle);
        drawCamLobe(fb, exCam.x, exCam.y, static_cast<float>(exCam.radius), exCam.angle);

        // 6. Reciprocating Poppet Valves with Compression Springs
        drawValve(fb, inValve.x, inValve.y, view.intakeLift, 55.0f, Color(100, 180, 240));
        drawValve(fb, exValve.x, exValve.y, view.exhaustLift, 55.0f, Color(240, 140, 80));

        // 7. Front Timing Sprockets
        drawGear(fb, crank);
        drawGear(fb, cam);
        drawPulley(fb, tens);

        // Sprocket Center Badges & Teeth Labels
        drawText(fb, ri(crank.x) - 18, ri(crank.y) + crank.radius + 6, "12T CRANK", 1, palette::glow);
        drawText(fb, ri(cam.x) - 16, ri(cam.y) - cam.radius - 14, "24T CAM", 1, palette::glow);

        // 8. Live Digital ECU Dashboard / Telemetry
        drawEngineDashboard(fb, view);

        // 9. Failure State: Interference Warning Banner
        if (view.collision) {
            fillRect(fb, 260, 270, 480, 50, Color(220, 40, 40));
            drawRect(fb, 260, 270, 480, 50, Color(255, 230, 80));
            drawText(fb, 280, 285, "VALVE-TO-PISTON INTERFERENCE CRASH!", 2, Color(255, 255, 255));
        }

        // 10. Selected Part Highlight Ring
        if (view.selectedPart >= 0 && view.selectedPart < static_cast<int>(parts.size())) {
            const PickCircle pc = pickCircle(parts[view.selectedPart]);
            const Color ringCol = view.beltTensioned ? Color(100, 240, 140) : Color(255, 80, 80);
            for (int k = 4; k <= 6; ++k) drawCircle(fb, pc.x, pc.y, pc.r + k, ringCol);
        }
        return;
    }

    // =========================================================================
    // CLASSIC CP1/CP2 MACHINE RENDERING
    // =========================================================================
    const float power = (view.powerLevel >= 0.0f) ? view.powerLevel : (view.powered ? 1.0f : 0.0f);
    const bool isLit = power > 0.05f;

    if (parts.size() < kPartCount) return;

    const Component& motor = parts[kPartMotor];
    const Component& gear1 = parts[kPartGear1];
    const Component& gear2 = parts[kPartGear2];
    const Component& pulley = parts[kPartPulley];
    const Component& wheel = parts[kPartWheel];
    const Component& bulb = parts[kPartBulb];

    drawShaft(fb, motor, gear1);
    drawShaft(fb, gear2, pulley);

    const float beltDist = std::hypot(pulley.x - wheel.x, pulley.y - wheel.y);
    if (beltDist > static_cast<float>(pulley.radius + wheel.radius) && beltDist < 600.0f) {
        drawBelt(fb, pulley.x, pulley.y, pulley.radius, wheel.x, wheel.y, wheel.radius, 4, palette::belt);
    }

    const float wireDist = std::hypot(wheel.x - bulb.x, wheel.y - (bulb.y + bulb.radius * 1.3f));
    if (wireDist < 350.0f) {
        if (isLit) {
            drawThickLine(fb, ri(wheel.x), ri(wheel.y), ri(bulb.x), ri(bulb.y + bulb.radius * 1.3f), 6,
                          Color(180, 110, 30));
            drawThickLine(fb, ri(wheel.x), ri(wheel.y), ri(bulb.x), ri(bulb.y + bulb.radius * 1.3f), 2,
                          Color(255, 220, 100));
        } else {
            drawThickLine(fb, ri(wheel.x), ri(wheel.y), ri(bulb.x), ri(bulb.y + bulb.radius * 1.3f), 5,
                          palette::metalDark);
        }
    }

    drawMotor(fb, motor);
    drawGear(fb, gear2);
    drawGear(fb, gear1);
    drawPulley(fb, pulley);
    drawPulley(fb, wheel);

    if (isLit) {
        const int glowRadius = static_cast<int>(40.0f + 90.0f * std::sqrt(power));
        drawGlow(fb, ri(bulb.x), ri(bulb.y), glowRadius, palette::glow, power);
    }

    Component drawnBulb = bulb;
    drawnBulb.lit = isLit;
    drawBulb(fb, drawnBulb);

    std::vector<Light> lights;
    if (isLit) {
        lights.push_back({bulb.x, bulb.y, 560.0f, power});
    }
    const float darkness = 0.60f - 0.20f * power;
    applyDarkness(fb, kToolbarWidth, kTopBarHeight, fb.width() - kToolbarWidth,
                  fb.height() - kTopBarHeight, darkness, lights, palette::glow);

    if (view.selectedPart >= 0 && view.selectedPart < static_cast<int>(parts.size())) {
        const PickCircle pc = pickCircle(parts[view.selectedPart]);
        const Color ringCol = view.isMeshed ? Color(100, 240, 140) : palette::glow;
        for (int k = 6; k <= 8; ++k) drawCircle(fb, pc.x, pc.y, pc.r + k, ringCol);
    }
}

void renderInterface(Framebuffer& fb, const SceneView& view) {
    drawToolbar(fb, view.hoveredSlot, view.isEngineMode);
    drawTopBar(fb, view.hoveredButton, view.isEngineMode ? "ENGINE" : "CLASSIC");
}

}  // namespace lumina
