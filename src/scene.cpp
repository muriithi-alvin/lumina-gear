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

// Blueprint target ghost markers for building from scratch
void drawBlueprintGuides(Framebuffer& fb, const std::vector<Component>& parts) {
    const struct Ghost { int idx; int x, y, r; const char* name; } kGhosts[] = {
        {kEngineCrank, 240, 460, 39, "[1] CRANK SPROCKET"},
        {kEngineCam, 240, 160, 45, "[2] CAM SPROCKET"},
        {kEngineTensioner, 340, 310, 24, "[3] BELT TENSIONER"},
        {kEngineValvetrain, 640, 260, 45, "[4] 4-STROKE CYLINDER"},
        {kEngineAlternator, 440, 460, 28, "[5] ALTERNATOR DYNAMO"},
        {kEngineBulb, 820, 270, 38, "[6] WORKSHOP BULB"}
    };

    for (const auto& g : kGhosts) {
        if (g.idx < static_cast<int>(parts.size()) && !parts[g.idx].placed) {
            // Cyan dashed ghost circle
            for (int i = 0; i < 24; ++i) {
                if (i % 2 == 0) {
                    const float a0 = i * 2.0f * kPi / 24.0f;
                    const float a1 = (i + 1) * 2.0f * kPi / 24.0f;
                    drawLine(fb, ri(g.x + g.r * std::cos(a0)), ri(g.y + g.r * std::sin(a0)),
                             ri(g.x + g.r * std::cos(a1)), ri(g.y + g.r * std::sin(a1)), palette::blueprintCyan);
                }
            }
            fillCircle(fb, g.x, g.y, 3, palette::blueprintCyan);
            drawText(fb, g.x - 45, g.y + g.r + 6, g.name, 1, palette::blueprintCyan);
        }
    }
}

void drawEngineBlock(Framebuffer& fb, const SceneView& view) {
    const int bLeft = 530, bRight = 750;
    const int bTop = 230, bBot = 520;
    fillRect(fb, bLeft, bTop, bRight - bLeft, bBot - bTop, Color(38, 48, 56));
    drawRect(fb, bLeft, bTop, bRight - bLeft, bBot - bTop, palette::outline);

    fillRect(fb, 520, 110, 240, 70, Color(48, 60, 70));
    drawRect(fb, 520, 110, 240, 70, palette::outline);

    for (int bx : {540, 600, 680, 740}) {
        fillCircle(fb, bx, 125, 4, palette::metalLight);
        drawCircle(fb, bx, 125, 4, palette::outline);
    }

    const Color intakeCol = (view.stroke == 1 && view.rpm > 10.0f) ? palette::intakeAir : Color(60, 80, 95);
    drawThickLine(fb, 470, 200, 560, 245, 14, intakeCol);
    drawThickLine(fb, 470, 200, 560, 245, 2, palette::outline);

    if (view.stroke == 1 && view.rpm > 10.0f) {
        drawText(fb, 450, 175, "AIR/FUEL", 1, palette::intakeAir);
        for (int p = 0; p < 4; ++p) {
            fillCircle(fb, 485 + p * 20, 208 + p * 10, 3, Color(160, 220, 255));
        }
    }

    const Color exhaustCol = (view.stroke == 4 && view.rpm > 10.0f) ? palette::exhaustGas : Color(70, 55, 50);
    drawThickLine(fb, 720, 245, 810, 200, 14, exhaustCol);
    drawThickLine(fb, 720, 245, 810, 200, 2, palette::outline);

    if (view.stroke == 4 && view.rpm > 10.0f) {
        drawText(fb, 760, 175, "EXHAUST", 1, palette::exhaustGas);
        for (int p = 0; p < 4; ++p) {
            fillCircle(fb, 735 + p * 20, 237 - p * 10, 4, Color(255, 140, 40));
        }
    }

    // Spark plug
    fillRect(fb, 636, 185, 8, 30, Color(240, 240, 245));
    fillRect(fb, 634, 215, 12, 12, palette::metalLight);
    drawRect(fb, 634, 185, 12, 42, palette::outline);
    drawLine(fb, 640, 227, 640, 244, palette::metalDark);

    if (view.combustion) {
        drawThickLine(fb, 638, 243, 642, 243, 3, Color(255, 255, 255));
        drawGlow(fb, 640, 243, 18, Color(100, 200, 255), 1.0f);
    }
}

void drawTimingBelt(Framebuffer& fb, const Component& crank, const Component& cam, const Component& tensioner, bool tensioned) {
    if (!crank.placed || !cam.placed || !tensioner.placed) return;

    const Color beltBase = tensioned ? palette::belt : Color(90, 75, 65);
    const Color toothCol = Color(40, 35, 30);

    const float camLeftX = cam.x - cam.radius + 3;
    const float crankLeftX = crank.x - crank.radius + 3;
    drawThickLine(fb, ri(camLeftX), ri(cam.y), ri(crankLeftX), ri(crank.y), 8, beltBase);
    drawLine(fb, ri(camLeftX) - 4, ri(cam.y), ri(crankLeftX) - 4, ri(crank.y), palette::outline);
    drawLine(fb, ri(camLeftX) + 4, ri(cam.y), ri(crankLeftX) + 4, ri(crank.y), palette::outline);

    for (int y = ri(cam.y); y <= ri(crank.y); y += 12) {
        const float t = static_cast<float>(y - cam.y) / (crank.y - cam.y);
        const int x = ri(camLeftX + t * (crankLeftX - camLeftX));
        drawLine(fb, x - 2, y, x + 2, y, toothCol);
    }

    const float crankRightX = crank.x + crank.radius * 0.7f;
    const float crankRightY = crank.y + crank.radius * 0.7f;
    drawThickLine(fb, ri(crankRightX), ri(crankRightY), ri(tensioner.x), ri(tensioner.y + tensioner.radius), 8, beltBase);
    drawLine(fb, ri(crankRightX), ri(crankRightY) + 4, ri(tensioner.x), ri(tensioner.y + tensioner.radius) + 4, palette::outline);

    const float camRightX = cam.x + cam.radius * 0.85f;
    const float camRightY = cam.y + cam.radius * 0.5f;
    drawThickLine(fb, ri(tensioner.x), ri(tensioner.y - tensioner.radius), ri(camRightX), ri(camRightY), 8, beltBase);
    drawLine(fb, ri(tensioner.x) + 4, ri(tensioner.y - tensioner.radius), ri(camRightX) + 4, ri(camRightY), palette::outline);

    if (tensioned) {
        drawText(fb, ri(tensioner.x) + 26, ri(tensioner.y) - 6, "TENSION: OK", 1, Color(100, 240, 140));
    } else {
        drawText(fb, ri(tensioner.x) + 26, ri(tensioner.y) - 6, "BELT SLACK", 1, Color(255, 90, 80));
    }

    drawLine(fb, ri(crank.x), ri(crank.y - crank.radius - 8), ri(crank.x), ri(crank.y - crank.radius), Color(255, 220, 60));
    drawLine(fb, ri(cam.x), ri(cam.y - cam.radius - 8), ri(cam.x), ri(cam.y - cam.radius), Color(255, 220, 60));
}

// Drive belt connecting Crankshaft to Alternator generator
void drawAlternatorDriveBelt(Framebuffer& fb, const Component& crank, const Component& alt, bool driven) {
    if (!crank.placed || !alt.placed) return;
    const Color bCol = driven ? Color(55, 60, 65) : Color(90, 80, 75);
    drawBelt(fb, crank.x, crank.y, 22, alt.x, alt.y, alt.radius, 4, bCol);
}

// Heavy copper electrical wiring from Alternator to Lightbulb
void drawElectricalCircuit(Framebuffer& fb, const Component& alt, const Component& bulb, bool powered) {
    if (!alt.placed || !bulb.placed) return;

    const int ax = ri(alt.x), ay = ri(alt.y);
    const int bx = ri(bulb.x), by = ri(bulb.y + bulb.radius * 1.2f);

    // Negative lead wire (black ground wire along bottom)
    const int gndY = 540;
    drawThickLine(fb, ax + 12, ay - alt.radius - 4, ax + 35, gndY, 3, Color(28, 32, 36));
    drawThickLine(fb, ax + 35, gndY, bx + 10, gndY, 3, Color(28, 32, 36));
    drawThickLine(fb, bx + 10, gndY, bx + 10, by, 3, Color(28, 32, 36));

    // Positive lead wire (copper power wire)
    const Color wireCol = powered ? palette::wireGlow : palette::wireCopper;
    drawThickLine(fb, ax - 12, ay - alt.radius - 4, ax + 20, 520, 4, wireCol);
    drawThickLine(fb, ax + 20, 520, bx - 10, 520, 4, wireCol);
    drawThickLine(fb, bx - 10, 520, bx - 10, by, 4, wireCol);

    // Flowing electrical current pulse dots when power flows
    if (powered) {
        for (int p = 0; p < 8; ++p) {
            const int dotX = ri(ax + 30 + p * ((bx - ax - 40) / 8.0f));
            fillCircle(fb, dotX, 520, 3, Color(255, 255, 180));
        }
        drawText(fb, ri((ax + bx) * 0.5f) - 30, 502, "14.4V DC POWER FLOW", 1, palette::glow);
    }
}

void drawEngineDashboard(Framebuffer& fb, const SceneView& view) {
    const int x0 = 760, y0 = 56, w = 224, h = 180;

    fillRect(fb, x0, y0, w, h, Color(24, 32, 38));
    drawRect(fb, x0, y0, w, h, Color(60, 80, 95));
    drawRect(fb, x0 + 1, y0 + 1, w - 2, h - 2, palette::outline);

    drawText(fb, x0 + 12, y0 + 8, "SYSTEM TELEMETRY", 1, palette::glow);
    drawLine(fb, x0 + 10, y0 + 20, x0 + w - 10, y0 + 20, Color(50, 65, 75));

    char buf[40];
    std::snprintf(buf, sizeof(buf), "ENGINE RPM: %4d", ri(view.rpm));
    drawText(fb, x0 + 12, y0 + 26, buf, 1, Color(230, 240, 245));

    const int barW = w - 24;
    const int rpmFill = std::clamp(ri(barW * (view.rpm / 2000.0f)), 0, barW);
    fillRect(fb, x0 + 12, y0 + 38, barW, 6, Color(40, 50, 60));
    fillRect(fb, x0 + 12, y0 + 38, rpmFill, 6, view.rpm > 1600 ? Color(255, 90, 80) : Color(100, 220, 140));
    drawRect(fb, x0 + 12, y0 + 38, barW, 6, palette::outline);

    const char* sName = "IDLE";
    Color sColor = Color(180, 190, 200);
    switch (view.stroke) {
        case 1: sName = "STROKE: INTAKE"; sColor = palette::intakeAir; break;
        case 2: sName = "STROKE: COMPRESSION"; sColor = Color(120, 200, 240); break;
        case 3: sName = "STROKE: POWER FIRE"; sColor = palette::combustionFire; break;
        case 4: sName = "STROKE: EXHAUST"; sColor = palette::exhaustGas; break;
    }
    drawText(fb, x0 + 12, y0 + 50, sName, 1, sColor);

    if (view.collision) {
        drawText(fb, x0 + 12, y0 + 68, "[COLLISION CRASH!]", 1, Color(255, 60, 60));
    } else if (view.beltTensioned) {
        drawText(fb, x0 + 12, y0 + 68, "TIMING: 2:1 SYNC [OK]", 1, Color(100, 240, 140));
    } else {
        drawText(fb, x0 + 12, y0 + 68, "TIMING: BELT SLACK", 1, Color(255, 140, 50));
    }

    std::snprintf(buf, sizeof(buf), "ALT RPM:   %4d", ri(view.alternatorRpm));
    drawText(fb, x0 + 12, y0 + 86, buf, 1, Color(190, 210, 225));

    std::snprintf(buf, sizeof(buf), "VOLTAGE:   %2.1f V", view.voltage);
    drawText(fb, x0 + 12, y0 + 102, buf, 1, view.voltage > 11.0f ? Color(100, 240, 140) : Color(180, 190, 200));

    std::snprintf(buf, sizeof(buf), "BULB PWR:  %3d %%", ri(view.powerLevel * 100.0f));
    drawText(fb, x0 + 12, y0 + 120, buf, 1, view.powered ? palette::glow : Color(160, 170, 180));

    std::snprintf(buf, sizeof(buf), "OUTPUT:    %3d HP", ri(view.hp));
    drawText(fb, x0 + 12, y0 + 138, buf, 1, palette::glow);

    // Assembly completion indicator
    if (view.assemblyComplete) {
        drawText(fb, x0 + 12, y0 + 158, "CIRCUIT CLOSED [100%]", 1, Color(100, 240, 140));
    } else {
        drawText(fb, x0 + 12, y0 + 158, "ASSEMBLY IN PROGRESS", 1, Color(255, 200, 80));
    }
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
        if (!parts[i].placed) continue;
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
    // ENGINE-TO-ELECTRICITY VALVETRAIN ASSEMBLY
    // =========================================================================
    if (view.isEngineMode) {
        if (parts.size() < kEnginePartCount) return;

        const Component& crank = parts[kEngineCrank];
        const Component& cam = parts[kEngineCam];
        const Component& tens = parts[kEngineTensioner];
        const Component& vtrain = parts[kEngineValvetrain];
        const Component& alt = parts[kEngineAlternator];
        const Component& bulb = parts[kEngineBulb];
        const Component& inCam = parts[kEngineIntakeCam];
        const Component& exCam = parts[kEngineExhaustCam];
        const Component& inValve = parts[kEngineIntakeValve];
        const Component& exValve = parts[kEngineExhaustValve];
        const Component& piston = parts[kEnginePiston];

        // 1. Blueprint ghost guide markers for unplaced components
        drawBlueprintGuides(fb, parts);

        // 2. Engine Block & Manifolds (if cylinder placed)
        if (vtrain.placed) {
            drawEngineBlock(fb, view);

            // Overhead camshaft
            drawThickLine(fb, ri(cam.x), ri(cam.y), 745, ri(cam.y), 10, palette::metalMid);
            drawThickLine(fb, ri(cam.x), ri(cam.y) - 2, 745, ri(cam.y) - 2, 3, palette::metalLight);
            fillRect(fb, 410, ri(cam.y) - 10, 24, 20, palette::metalDark);
            drawRect(fb, 410, ri(cam.y) - 10, 24, 20, palette::outline);
            fillRect(fb, 735, ri(cam.y) - 10, 20, 20, palette::metalDark);
            drawRect(fb, 735, ri(cam.y) - 10, 20, 20, palette::outline);

            // Cylinder bore, piston, conrod
            drawPistonAndCylinder(fb, piston.x, piston.y, 140.0f, 190.0f,
                                  view.pistonY, view.rodAngle, view.combustion);

            // Cam lobes & valves
            drawCamLobe(fb, inCam.x, inCam.y, static_cast<float>(inCam.radius), inCam.angle);
            drawCamLobe(fb, exCam.x, exCam.y, static_cast<float>(exCam.radius), exCam.angle);
            drawValve(fb, inValve.x, inValve.y, view.intakeLift, 55.0f, Color(100, 180, 240));
            drawValve(fb, exValve.x, exValve.y, view.exhaustLift, 55.0f, Color(240, 140, 80));
        }

        // 3. Timing Belt
        drawTimingBelt(fb, crank, cam, tens, view.beltTensioned);

        // 4. Alternator Drive Belt
        drawAlternatorDriveBelt(fb, crank, alt, view.alternatorDriven);

        // 5. Electrical Wiring Circuit
        drawElectricalCircuit(fb, alt, bulb, view.powered);

        // 6. Front Sprockets & Pulleys
        if (crank.placed) {
            drawGear(fb, crank);
            drawText(fb, ri(crank.x) - 18, ri(crank.y) + crank.radius + 6, "12T CRANK", 1, palette::glow);
        }
        if (cam.placed) {
            drawGear(fb, cam);
            drawText(fb, ri(cam.x) - 16, ri(cam.y) - cam.radius - 14, "24T CAM", 1, palette::glow);
        }
        if (tens.placed) {
            drawPulley(fb, tens);
        }
        if (alt.placed) {
            drawAlternator(fb, alt, view.alternatorAngle, view.alternatorDriven && view.rpm > 10.0f);
            drawText(fb, ri(alt.x) - 18, ri(alt.y) + alt.radius + 6, "12V DYNAMO", 1, Color(140, 220, 255));
        }

        // 7. Lightbulb & Dynamic Room Lighting
        if (bulb.placed) {
            if (view.powered) {
                const int glowRadius = static_cast<int>(50.0f + 110.0f * std::sqrt(view.powerLevel));
                drawGlow(fb, ri(bulb.x), ri(bulb.y), glowRadius, palette::glow, view.powerLevel);
            }
            Component drawnBulb = bulb;
            drawnBulb.lit = view.powered;
            drawBulb(fb, drawnBulb);
            drawText(fb, ri(bulb.x) - 16, ri(bulb.y) + bulb.radius + 18, "100W LAMP", 1, palette::glow);
        }

        // Dynamic darkness attenuation
        std::vector<Light> lights;
        if (view.powered && bulb.placed) {
            lights.push_back({bulb.x, bulb.y, 600.0f, view.powerLevel});
        }
        const float darkness = 0.60f - 0.25f * (view.powered ? view.powerLevel : 0.0f);
        applyDarkness(fb, kToolbarWidth, kTopBarHeight, fb.width() - kToolbarWidth,
                      fb.height() - kTopBarHeight, darkness, lights, palette::glow);

        // 8. Mission Objective Banner on Top
        if (!view.objective.empty()) {
            const int bannerW = textWidth(view.objective, 1) + 20;
            const int bannerX = kToolbarWidth + 14;
            fillRect(fb, bannerX, kTopBarHeight + 8, bannerW, 18, Color(24, 34, 42));
            drawRect(fb, bannerX, kTopBarHeight + 8, bannerW, 18, palette::blueprintCyan);
            drawText(fb, bannerX + 8, kTopBarHeight + 13, view.objective, 1, Color(160, 230, 255));
        }

        // 9. Dashboard Telemetry
        drawEngineDashboard(fb, view);

        // 10. Crash Warning
        if (view.collision) {
            fillRect(fb, 260, 270, 480, 50, Color(220, 40, 40));
            drawRect(fb, 260, 270, 480, 50, Color(255, 230, 80));
            drawText(fb, 280, 285, "VALVE-TO-PISTON INTERFERENCE CRASH!", 2, Color(255, 255, 255));
        }

        // 11. Selection Highlight Ring
        if (view.selectedPart >= 0 && view.selectedPart < static_cast<int>(parts.size()) && parts[view.selectedPart].placed) {
            const PickCircle pc = pickCircle(parts[view.selectedPart]);
            const Color ringCol = view.beltTensioned ? Color(100, 240, 140) : Color(255, 80, 80);
            for (int k = 4; k <= 6; ++k) drawCircle(fb, pc.x, pc.y, pc.r + k, ringCol);
        }
        return;
    }

    // =========================================================================
    // CLASSIC CP1/CP2 MACHINE
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
            drawThickLine(fb, ri(wheel.x), ri(wheel.y), ri(bulb.x), ri(bulb.y + bulb.radius * 1.3f), 6, Color(180, 110, 30));
            drawThickLine(fb, ri(wheel.x), ri(wheel.y), ri(bulb.x), ri(bulb.y + bulb.radius * 1.3f), 2, Color(255, 220, 100));
        } else {
            drawThickLine(fb, ri(wheel.x), ri(wheel.y), ri(bulb.x), ri(bulb.y + bulb.radius * 1.3f), 5, palette::metalDark);
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
    if (isLit) lights.push_back({bulb.x, bulb.y, 560.0f, power});
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

    // TinkerCAD / Wokwi hover info card for toolbar slot
    if (view.hoveredSlot >= 0 && view.hoveredSlot < kSlotCount) {
        drawComponentInfoCard(fb, view.hoveredSlot, view.mouseX, view.mouseY);
    }
}

}  // namespace lumina
