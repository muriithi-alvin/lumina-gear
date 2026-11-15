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
    const Color dot(52, 68, 76);
    for (int y = kTopBarHeight + 20; y < fb.height(); y += 40)
        for (int x = kToolbarWidth + 20; x < fb.width(); x += 40) fb.setPixel(x, y, dot);
}

void drawShaft(Framebuffer& fb, const Component& a, const Component& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dist = std::hypot(dx, dy);
    if (dist > 360.0f) return;

    drawThickLine(fb, ri(a.x), ri(a.y), ri(b.x), ri(b.y), 8, palette::metalDark);
    drawThickLine(fb, ri(a.x), ri(a.y) - 2, ri(b.x), ri(b.y) - 2, 2, palette::metalLight);
}

struct PickCircle { int x, y, r; };

PickCircle pickCircle(const Component& c) {
    if (c.type == ComponentType::Motor) return {ri(c.x) - c.radius, ri(c.y), c.radius};
    return {ri(c.x), ri(c.y), c.radius};
}

void drawEngineBlock(Framebuffer& fb, const SceneView& view) {
    const int bLeft = 700, bRight = 980;
    const int bTop = 270, bBot = 620;
    fillRect(fb, bLeft, bTop, bRight - bLeft, bBot - bTop, Color(38, 48, 56));
    drawRect(fb, bLeft, bTop, bRight - bLeft, bBot - bTop, palette::outline);

    // Cylinder head cover
    fillRect(fb, 690, 140, 300, 80, Color(48, 60, 70));
    drawRect(fb, 690, 140, 300, 80, palette::outline);

    for (int bx : {715, 780, 890, 965}) {
        fillCircle(fb, bx, 158, 5, palette::metalLight);
        drawCircle(fb, bx, 158, 5, palette::outline);
    }

    // Intake runner
    const Color intakeCol = (view.stroke == 1 && view.rpm > 10.0f) ? palette::intakeAir : Color(60, 80, 95);
    drawThickLine(fb, 630, 240, 750, 290, 16, intakeCol);
    drawThickLine(fb, 630, 240, 750, 290, 2, palette::outline);

    if (view.stroke == 1 && view.rpm > 10.0f) {
        drawText(fb, 610, 215, "AIR/FUEL IN", 1, palette::intakeAir);
        for (int p = 0; p < 4; ++p) fillCircle(fb, 645 + p * 24, 248 + p * 10, 3, Color(160, 220, 255));
    }

    // Exhaust runner
    const Color exhaustCol = (view.stroke == 4 && view.rpm > 10.0f) ? palette::exhaustGas : Color(70, 55, 50);
    drawThickLine(fb, 930, 290, 1050, 240, 16, exhaustCol);
    drawThickLine(fb, 930, 290, 1050, 240, 2, palette::outline);

    if (view.stroke == 4 && view.rpm > 10.0f) {
        drawText(fb, 990, 215, "EXHAUST OUT", 1, palette::exhaustGas);
        for (int p = 0; p < 4; ++p) fillCircle(fb, 950 + p * 24, 280 - p * 10, 4, Color(255, 140, 40));
    }

    // Spark plug
    fillRect(fb, 836, 215, 8, 35, Color(240, 240, 245));
    fillRect(fb, 834, 250, 12, 14, palette::metalLight);
    drawRect(fb, 834, 215, 12, 49, palette::outline);
    drawLine(fb, 840, 264, 840, 280, palette::metalDark);

    if (view.combustion) {
        drawThickLine(fb, 838, 280, 842, 280, 3, Color(255, 255, 255));
        drawGlow(fb, 840, 280, 24, Color(100, 200, 255), 1.0f);
    }
}

void drawTimingBelt(Framebuffer& fb, const Component& crank, const Component& cam, const Component& tensioner, bool tensioned) {
    if (!crank.placed || !cam.placed || !tensioner.placed) return;

    const Color beltBase = tensioned ? palette::belt : Color(90, 75, 65);
    const Color toothCol = Color(40, 35, 30);

    const float camLeftX = cam.x - cam.radius + 3;
    const float crankLeftX = crank.x - crank.radius + 3;
    drawThickLine(fb, ri(camLeftX), ri(cam.y), ri(crankLeftX), ri(crank.y), 9, beltBase);
    drawLine(fb, ri(camLeftX) - 4, ri(cam.y), ri(crankLeftX) - 4, ri(crank.y), palette::outline);
    drawLine(fb, ri(camLeftX) + 4, ri(cam.y), ri(crankLeftX) + 4, ri(crank.y), palette::outline);

    for (int y = ri(cam.y); y <= ri(crank.y); y += 14) {
        const float t = static_cast<float>(y - cam.y) / (crank.y - cam.y);
        const int x = ri(camLeftX + t * (crankLeftX - camLeftX));
        drawLine(fb, x - 2, y, x + 2, y, toothCol);
    }

    const float crankRightX = crank.x + crank.radius * 0.7f;
    const float crankRightY = crank.y + crank.radius * 0.7f;
    drawThickLine(fb, ri(crankRightX), ri(crankRightY), ri(tensioner.x), ri(tensioner.y + tensioner.radius), 9, beltBase);
    drawLine(fb, ri(crankRightX), ri(crankRightY) + 4, ri(tensioner.x), ri(tensioner.y + tensioner.radius) + 4, palette::outline);

    const float camRightX = cam.x + cam.radius * 0.85f;
    const float camRightY = cam.y + cam.radius * 0.5f;
    drawThickLine(fb, ri(tensioner.x), ri(tensioner.y - tensioner.radius), ri(camRightX), ri(camRightY), 9, beltBase);
    drawLine(fb, ri(tensioner.x) + 4, ri(tensioner.y - tensioner.radius), ri(camRightX) + 4, ri(camRightY), palette::outline);

    if (tensioned) {
        drawText(fb, ri(tensioner.x) + 28, ri(tensioner.y) - 6, "TENSION: OK", 1, Color(100, 240, 140));
    } else {
        drawText(fb, ri(tensioner.x) + 28, ri(tensioner.y) - 6, "BELT SLACK", 1, Color(255, 90, 80));
    }

    drawLine(fb, ri(crank.x), ri(crank.y - crank.radius - 8), ri(crank.x), ri(crank.y - crank.radius), Color(255, 220, 60));
    drawLine(fb, ri(cam.x), ri(cam.y - cam.radius - 8), ri(cam.x), ri(cam.y - cam.radius), Color(255, 220, 60));
}

void drawAlternatorDriveBelt(Framebuffer& fb, const Component& crank, const Component& alt, bool driven) {
    if (!crank.placed || !alt.placed) return;
    const Color bCol = driven ? Color(55, 60, 65) : Color(90, 80, 75);
    drawBelt(fb, crank.x, crank.y, 22, alt.x, alt.y, alt.radius, 5, bCol);
}

void drawElectricalCircuit(Framebuffer& fb, const Component& alt, const Component& bulb, bool powered) {
    if (!alt.placed || !bulb.placed) return;

    const int ax = ri(alt.x), ay = ri(alt.y);
    const int bx = ri(bulb.x), by = ri(bulb.y + bulb.radius * 1.2f);

    const int gndY = 680;
    drawThickLine(fb, ax + 12, ay - alt.radius - 4, ax + 40, gndY, 3, Color(28, 32, 36));
    drawThickLine(fb, ax + 40, gndY, bx + 10, gndY, 3, Color(28, 32, 36));
    drawThickLine(fb, bx + 10, gndY, bx + 10, by, 3, Color(28, 32, 36));

    const Color wireCol = powered ? palette::wireGlow : palette::wireCopper;
    drawThickLine(fb, ax - 12, ay - alt.radius - 4, ax + 25, 650, 4, wireCol);
    drawThickLine(fb, ax + 25, 650, bx - 10, 650, 4, wireCol);
    drawThickLine(fb, bx - 10, 650, bx - 10, by, 4, wireCol);

    if (powered) {
        for (int p = 0; p < 10; ++p) {
            const int dotX = ri(ax + 35 + p * ((bx - ax - 50) / 10.0f));
            fillCircle(fb, dotX, 650, 3, Color(255, 255, 180));
        }
        drawText(fb, ri((ax + bx) * 0.5f) - 40, 630, "14.4V DC POWER FLOW", 1, palette::glow);
    }
}

void drawEngineDashboard(Framebuffer& fb, const SceneView& view) {
    const int x0 = 1040, y0 = 56, w = 300, h = 210;

    fillRect(fb, x0, y0, w, h, Color(22, 30, 36));
    drawRect(fb, x0, y0, w, h, Color(55, 75, 90));
    drawRect(fb, x0 + 1, y0 + 1, w - 2, h - 2, palette::outline);

    const char* dashTitle = (view.currentLevel == 4) ? "VALVETRAIN TIMING BENCH" : "MASTER POWER PLANT ECU";
    drawText(fb, x0 + 12, y0 + 8, dashTitle, 1, palette::glow);
    drawLine(fb, x0 + 10, y0 + 20, x0 + w - 10, y0 + 20, Color(45, 60, 70));

    char buf[64];
    std::snprintf(buf, sizeof(buf), "ENGINE RPM: %4d", ri(view.rpm));
    drawText(fb, x0 + 12, y0 + 26, buf, 1, Color(230, 240, 245));

    const int barW = w - 24;
    const int rpmFill = std::clamp(ri(barW * (view.rpm / 2000.0f)), 0, barW);
    fillRect(fb, x0 + 12, y0 + 38, barW, 6, Color(35, 45, 55));
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
        drawText(fb, x0 + 12, y0 + 68, "[CRASH: VALVE COLLISION!]", 1, Color(255, 60, 60));
    } else if (view.beltTensioned) {
        drawText(fb, x0 + 12, y0 + 68, "TIMING: 2:1 SYNC [OK]", 1, Color(100, 240, 140));
    } else {
        drawText(fb, x0 + 12, y0 + 68, "TIMING: BELT SLACK", 1, Color(255, 140, 50));
    }

    if (view.currentLevel == 4) {
        drawText(fb, x0 + 12, y0 + 88, "ALT/BULB:  N/A (BENCH TEST)", 1, Color(140, 160, 175));
    } else {
        std::snprintf(buf, sizeof(buf), "ALT RPM:   %4d", ri(view.alternatorRpm));
        drawText(fb, x0 + 12, y0 + 88, buf, 1, Color(190, 210, 225));
    }

    std::snprintf(buf, sizeof(buf), "VOLTAGE:   %2.1f V DC", view.voltage);
    drawText(fb, x0 + 12, y0 + 106, buf, 1, view.voltage > 11.0f ? Color(100, 240, 140) : Color(180, 190, 200));

    std::snprintf(buf, sizeof(buf), "BULB PWR:  %3d %%", ri(view.powerLevel * 100.0f));
    drawText(fb, x0 + 12, y0 + 124, buf, 1, view.powered ? palette::glow : Color(160, 170, 180));

    std::snprintf(buf, sizeof(buf), "OUTPUT:    %3d HP", ri(view.hp));
    drawText(fb, x0 + 12, y0 + 142, buf, 1, palette::glow);

    // Level status banner
    if (view.levelCompleted) {
        drawText(fb, x0 + 12, y0 + 164, "GOAL ACHIEVED! [CLEARED]", 1, Color(100, 240, 140));
    } else if (view.collision) {
        drawText(fb, x0 + 12, y0 + 164, "FAILED: RESOLVE COLLISION", 1, Color(255, 80, 80));
    } else {
        drawText(fb, x0 + 12, y0 + 164, "GOAL: HIT TARGET SPECS", 1, Color(255, 200, 80));
    }
}

}  // namespace

std::vector<Component> sceneParts(bool powered) {
    // Classic CP1/CP2 Machine Layout (Spaced comfortably on 1360x760)
    const float midY = 380;
    std::vector<Component> parts(kPartCount);
    parts[kPartMotor] = makeMotor(220, midY, 46);
    parts[kPartGear1] = makeGear(370, midY, 12, 0);
    const float gear2X = parts[kPartGear1].x + gearPitchRadius(12) + gearPitchRadius(18); // 370 + 90 = 460
    parts[kPartGear2] = makeGear(gear2X, midY, 18, meshAngle(18, kPi));
    parts[kPartPulley] = makePulley(gear2X + 180, midY, 40); // 640
    parts[kPartWheel] = makePulley(1080, 520, 22);
    parts[kPartBulb] = makeBulb(1080, 290, 44, powered);
    for (auto& p : parts) p.placed = true;
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

void getTipCardBounds(const std::vector<Component>& parts, const SceneView& view, int& x, int& y, int& w, int& h) {
    w = view.tipCardMinimized ? 190 : 660;
    h = view.tipCardMinimized ? 24 : 56;

    if (view.tipCardX >= 0 && view.tipCardY >= 0) {
        x = view.tipCardX;
        y = view.tipCardY;
    } else {
        // Auto-dodge: check if lower canvas (y > 480) has any placed component
        bool lowerOccupied = false;
        for (const auto& p : parts) {
            if (p.placed && p.y > 480.0f && p.x > 260.0f && p.x < 1150.0f) {
                lowerOccupied = true;
                break;
            }
        }
        if (lowerOccupied) {
            x = 380;
            y = 82;
        } else {
            x = 380;
            y = 620;
        }
    }
    x = std::clamp(x, kToolbarWidth + 10, kSceneWidth - w - 10);
    y = std::clamp(y, kTopBarHeight + 10, kSceneHeight - h - 24);
}

void drawTipCard(Framebuffer& fb, const std::vector<Component>& parts, const SceneView& view) {
    int cardX, cardY, cardW, cardH;
    getTipCardBounds(parts, view, cardX, cardY, cardW, cardH);

    Color bgCol = Color(20, 30, 38);
    Color borderCol = Color(70, 190, 220);
    Color line1Col = Color(160, 230, 255);
    Color line2Col = Color(255, 230, 120);

    const char* line1 = "";
    const char* line2 = "";

    if (view.levelCompleted) {
        bgCol = Color(20, 45, 30);
        borderCol = Color(80, 230, 120);
        line1Col = Color(120, 255, 160);
        line2Col = Color(255, 255, 255);
        line1 = "[GOAL SOLVED: 100%!] LEVEL COMPLETED!";
        line2 = "All mechanical & electrical specifications satisfied! Select next level.";
    } else {
        switch (view.currentLevel) {
            case 1:
                line1 = "[LEVEL 1: BASELINE LUMINA MACHINE]";
                line2 = "Motor (800 RPM) -> 12T -> 18T -> 40px -> Belt -> 22px -> Bulb. Hit PLAY!";
                break;
            case 2:
                line1 = "[LEVEL 2: OVERDRIVE STEP-UP CHALLENGE (TARGET >= 12.0V DC)]";
                line2 = "Motor runs at 400 RPM (7.2V DC). Mount 18T on Motor -> 12T on Pulley for 14.4V!";
                break;
            case 3:
                line1 = "[LEVEL 3: DIRECTION & POLARITY CHALLENGE]";
                line2 = "Generator turns CCW (-). Insert 14T Idler Gear or press 'B' to Cross Belt!";
                break;
            case 4:
                line1 = "[LEVEL 4: 4-STROKE VALVETRAIN TIMING BENCH]";
                line2 = "12T Cam runs 1:1 (crashes at TDC!). Mount 24T Cam Gear for 2:1 synchronized timing!";
                break;
            case 5:
                line1 = "[LEVEL 5: MASTER POWER PLANT - GRID INTEGRATION]";
                line2 = "Engine is timed. Mount 12V Dynamo (Slot 6) & 100W Lamp (Slot 7) to generate power!";
                break;
        }
    }

    fillRect(fb, cardX, cardY, cardW, cardH, bgCol);
    drawRect(fb, cardX, cardY, cardW, cardH, borderCol);

    if (view.tipCardMinimized) {
        drawText(fb, cardX + 12, cardY + 7, "[?] TIPS [EXPAND +]", 1, line1Col);
    } else {
        fillRect(fb, cardX + 1, cardY + 1, cardW - 2, 16, Color(30, 42, 52));
        drawLine(fb, cardX + 1, cardY + 17, cardX + cardW - 2, cardY + 17, borderCol);
        drawText(fb, cardX + 8, cardY + 4, ":: MODULAR MISSION BRIEFING :: (DRAG TO MOVE)", 1, Color(140, 180, 205));
        drawText(fb, cardX + cardW - 52, cardY + 4, "[HIDE -]", 1, Color(255, 200, 100));

        drawText(fb, cardX + 12, cardY + 22, line1, 1, line1Col);
        drawText(fb, cardX + 12, cardY + 38, line2, 1, line2Col);
    }
}

void renderCanvas(Framebuffer& fb, const std::vector<Component>& parts, const SceneView& view) {
    fb.clear(palette::background);
    drawGrid(fb);

    // =========================================================================
    // LEVEL 4 & 5: 4-STROKE ENGINE VALVETRAIN & ELECTRICITY
    // =========================================================================
    if (view.currentLevel >= 4) {
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

        if (vtrain.placed) {
            drawEngineBlock(fb, view);

            // Overhead camshaft
            drawThickLine(fb, ri(cam.x), ri(cam.y), 960, ri(cam.y), 10, palette::metalMid);
            drawThickLine(fb, ri(cam.x), ri(cam.y) - 2, 960, ri(cam.y) - 2, 3, palette::metalLight);
            fillRect(fb, 580, ri(cam.y) - 10, 24, 20, palette::metalDark);
            drawRect(fb, 580, ri(cam.y) - 10, 24, 20, palette::outline);
            fillRect(fb, 950, ri(cam.y) - 10, 20, 20, palette::metalDark);
            drawRect(fb, 950, ri(cam.y) - 10, 20, 20, palette::outline);

            // Piston & Cylinder
            drawPistonAndCylinder(fb, piston.x, piston.y, 160.0f, 220.0f,
                                  view.pistonY, view.rodAngle, view.combustion);

            // Cam lobes & valves
            drawCamLobe(fb, inCam.x, inCam.y, static_cast<float>(inCam.radius), inCam.angle);
            drawCamLobe(fb, exCam.x, exCam.y, static_cast<float>(exCam.radius), exCam.angle);
            drawValve(fb, inValve.x, inValve.y, view.intakeLift, 65.0f, Color(100, 180, 240));
            drawValve(fb, exValve.x, exValve.y, view.exhaustLift, 65.0f, Color(240, 140, 80));
        }

        drawTimingBelt(fb, crank, cam, tens, view.beltTensioned);
        drawAlternatorDriveBelt(fb, crank, alt, view.alternatorDriven);
        drawElectricalCircuit(fb, alt, bulb, view.powered);

        if (crank.placed) {
            drawGear(fb, crank);
            drawText(fb, ri(crank.x) - 18, ri(crank.y) + crank.radius + 6, "12T CRANK", 1, palette::glow);
        }
        if (cam.placed) {
            drawGear(fb, cam);
            char cBuf[16];
            std::snprintf(cBuf, sizeof(cBuf), "%dT CAM", cam.teeth);
            drawText(fb, ri(cam.x) - 16, ri(cam.y) - cam.radius - 14, cBuf, 1, palette::glow);
        }
        if (tens.placed) {
            drawPulley(fb, tens);
        }
        if (alt.placed) {
            drawAlternator(fb, alt, view.alternatorAngle, view.alternatorDriven && view.rpm > 10.0f);
            drawText(fb, ri(alt.x) - 18, ri(alt.y) + alt.radius + 6, "12V DYNAMO", 1, Color(140, 220, 255));
        }

        if (bulb.placed) {
            if (view.powered) {
                const int glowRadius = static_cast<int>(55.0f + 130.0f * std::sqrt(view.powerLevel));
                drawGlow(fb, ri(bulb.x), ri(bulb.y), glowRadius, palette::glow, view.powerLevel);
            }
            Component drawnBulb = bulb;
            drawnBulb.lit = view.powered;
            drawBulb(fb, drawnBulb);
            drawText(fb, ri(bulb.x) - 18, ri(bulb.y) + bulb.radius + 18, "100W LAMP", 1, palette::glow);
        }

        // Dynamic light attenuation
        std::vector<Light> lights;
        if (view.powered && bulb.placed) {
            lights.push_back({bulb.x, bulb.y, 700.0f, view.powerLevel});
        }
        const float darkness = 0.60f - 0.25f * (view.powered ? view.powerLevel : 0.0f);
        applyDarkness(fb, kToolbarWidth, kTopBarHeight, fb.width() - kToolbarWidth,
                      fb.height() - kTopBarHeight, darkness, lights, palette::glow);

        // Mission objective banner on top
        if (!view.objective.empty()) {
            const int bW = textWidth(view.objective, 1) + 20;
            fillRect(fb, kToolbarWidth + 14, kTopBarHeight + 8, bW, 20, Color(20, 30, 38));
            drawRect(fb, kToolbarWidth + 14, kTopBarHeight + 8, bW, 20, Color(70, 190, 220));
            drawText(fb, kToolbarWidth + 22, kTopBarHeight + 14, view.objective, 1, Color(160, 230, 255));
        }

        drawEngineDashboard(fb, view);

        // =====================================================================
        // FATAL VALVE COLLISION ALERT & SPARK BURST!
        // =====================================================================
        if (view.collision) {
            fillRect(fb, 360, 320, 620, 60, Color(220, 35, 35));
            drawRect(fb, 360, 320, 620, 60, Color(255, 230, 80));
            drawText(fb, 380, 335, "FATAL CRASH: PISTON STRUCK VALVE AT TDC!", 2, Color(255, 255, 255));
            drawText(fb, 380, 358, "12T CAM GEAR RUNS AT 1:1! DRAG 24T CAM SPROCKET FROM TOOLBOX (2:1)", 1, Color(255, 240, 150));

            // Collision impact sparks
            for (int s = 0; s < 12; ++s) {
                const float a = s * 2.0f * kPi / 12.0f;
                const int sx = ri(840.0f + 25.0f * std::cos(a));
                const int sy = ri(285.0f + 25.0f * std::sin(a));
                drawLine(fb, 840, 285, sx, sy, Color(255, 220, 50));
            }
        }

        if (view.currentLevel == 4 && !view.collision && !view.levelCompleted) {
            fillRect(fb, 440, 610, 760, 50, Color(22, 32, 40));
            drawRect(fb, 440, 610, 760, 50, Color(70, 190, 220));
            drawText(fb, 455, 620, "[LEVEL 4: VALVETRAIN TIMING TEST BENCH (1:1 CRASH TEST)]", 1, Color(160, 230, 255));
            drawText(fb, 455, 638, "Equipped with 12T Cam Gear (1:1). HIT PLAY (SPACE) TO RUN TIMING TEST!", 1, Color(255, 230, 120));
        } else if (view.currentLevel == 4 && view.levelCompleted) {
            fillRect(fb, 440, 610, 760, 50, Color(20, 45, 30));
            drawRect(fb, 440, 610, 760, 50, Color(80, 230, 120));
            drawText(fb, 455, 620, "[TIMING SYNCHRONIZED!] 2:1 Reduction Ratio Confirmed!", 1, Color(120, 255, 160));
            drawText(fb, 455, 638, "Camshaft rotates at half engine speed. Zero valve collisions! Level 4 Cleared!", 1, Color(255, 255, 255));
        }

        if (view.currentLevel == 5 && (!alt.placed || !bulb.placed)) {
            fillRect(fb, 440, 610, 760, 50, Color(22, 32, 40));
            drawRect(fb, 440, 610, 760, 50, Color(70, 190, 220));
            drawText(fb, 455, 620, "[LEVEL 5: MASTER POWER PLANT - ELECTRICAL GRID UNBUILT]", 1, Color(160, 230, 255));
            drawText(fb, 455, 638, "Mount 12V Dynamo (Slot 6, next to Crank) & 100W Lamp (Slot 7) to generate power!", 1, Color(255, 230, 120));
        } else if (view.currentLevel == 5 && view.levelCompleted) {
            fillRect(fb, 440, 610, 760, 50, Color(20, 45, 30));
            drawRect(fb, 440, 610, 760, 50, Color(80, 230, 120));
            drawText(fb, 455, 620, "[POWER PLANT ONLINE!] All systems synchronized! 14.4V DC Grid Active!", 1, Color(120, 255, 160));
            drawText(fb, 455, 638, "Engine generates 185 HP, driving alternator to light 100W Lamp. Level 5 Cleared!", 1, Color(255, 255, 255));
        }

        if (view.selectedPart >= 0 && view.selectedPart < static_cast<int>(parts.size()) && parts[view.selectedPart].placed) {
            const PickCircle pc = pickCircle(parts[view.selectedPart]);
            const Color ringCol = view.beltTensioned ? Color(100, 240, 140) : Color(255, 80, 80);
            for (int k = 4; k <= 6; ++k) drawCircle(fb, pc.x, pc.y, pc.r + k, ringCol);
        }
        return;
    }

    // =========================================================================
    // LEVEL 1 TO 3: CLASSIC LUMINA MACHINE (ORIGINAL PROJECT)
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

    // Draw shaft from motor to whichever gear is placed near the motor
    int motorGear = -1;
    float bestMotorDist = 260.0f;
    for (int g : {kPartGear1, kPartGear2}) {
        if (parts[g].placed) {
            const float d = std::hypot(motor.x - parts[g].x, motor.y - parts[g].y);
            if (d <= bestMotorDist) {
                bestMotorDist = d;
                motorGear = g;
            }
        }
    }
    if (motor.placed && motorGear >= 0) {
        drawShaft(fb, motor, parts[motorGear]);
    }

    // Draw shaft from whichever gear is placed near pulley
    int pulleyGear = -1;
    float bestPulleyDist = 320.0f;
    for (int g : {kPartGear1, kPartGear2}) {
        if (parts[g].placed && g != motorGear) {
            const float d = std::hypot(pulley.x - parts[g].x, pulley.y - parts[g].y);
            if (d <= bestPulleyDist) {
                bestPulleyDist = d;
                pulleyGear = g;
            }
        }
    }
    if (pulley.placed && pulleyGear >= 0) {
        drawShaft(fb, parts[pulleyGear], pulley);
    }

    if (pulley.placed && wheel.placed) {
        const float beltDist = std::hypot(pulley.x - wheel.x, pulley.y - wheel.y);
        if (beltDist > static_cast<float>(pulley.radius + wheel.radius) && beltDist < 700.0f) {
            if (view.crossedBelt) {
                drawCrossedBelt(fb, pulley.x, pulley.y, pulley.radius, wheel.x, wheel.y, wheel.radius, 5, palette::belt);
                const int bmx = ri((pulley.x + wheel.x) * 0.5f);
                const int bmy = ri((pulley.y + wheel.y) * 0.5f);
                drawText(fb, bmx - 48, bmy - 14, "CROSSED BELT [REV]", 1, Color(120, 220, 255));
            } else {
                drawBelt(fb, pulley.x, pulley.y, pulley.radius, wheel.x, wheel.y, wheel.radius, 5, palette::belt);
            }
        }
    }

    if (wheel.placed && bulb.placed) {
        const float wireDist = std::hypot(wheel.x - bulb.x, wheel.y - (bulb.y + bulb.radius * 1.3f));
        if (wireDist < 500.0f) {
            if (isLit) {
                drawThickLine(fb, ri(wheel.x), ri(wheel.y), ri(bulb.x), ri(bulb.y + bulb.radius * 1.3f), 6, Color(180, 110, 30));
                drawThickLine(fb, ri(wheel.x), ri(wheel.y), ri(bulb.x), ri(bulb.y + bulb.radius * 1.3f), 2, Color(255, 220, 100));
            } else {
                drawThickLine(fb, ri(wheel.x), ri(wheel.y), ri(bulb.x), ri(bulb.y + bulb.radius * 1.3f), 5, palette::metalDark);
            }
        }
    }

    if (motor.placed) drawMotor(fb, motor);
    if (gear2.placed) drawGear(fb, gear2);
    if (gear1.placed) drawGear(fb, gear1);
    if (parts.size() > kPartIdler && parts[kPartIdler].placed) {
        drawGear(fb, parts[kPartIdler]);
        drawText(fb, ri(parts[kPartIdler].x) - 18, ri(parts[kPartIdler].y) + parts[kPartIdler].radius + 6, "14T IDLER", 1, palette::glow);
    }
    if (pulley.placed) drawPulley(fb, pulley);
    if (wheel.placed) drawPulley(fb, wheel);

    if (bulb.placed) {
        if (isLit) {
            const int glowRadius = static_cast<int>(45.0f + 110.0f * std::sqrt(power));
            drawGlow(fb, ri(bulb.x), ri(bulb.y), glowRadius, palette::glow, power);
        }
        Component drawnBulb = bulb;
        drawnBulb.lit = isLit;
        drawBulb(fb, drawnBulb);
    }

    std::vector<Light> lights;
    if (isLit && bulb.placed) lights.push_back({bulb.x, bulb.y, 650.0f, power});
    const float darkness = 0.60f - 0.20f * power;
    applyDarkness(fb, kToolbarWidth, kTopBarHeight, fb.width() - kToolbarWidth,
                  fb.height() - kTopBarHeight, darkness, lights, palette::glow);

    // Objective banner on top
    if (!view.objective.empty()) {
        const int bW = textWidth(view.objective, 1) + 20;
        fillRect(fb, kToolbarWidth + 14, kTopBarHeight + 8, bW, 20, Color(20, 30, 38));
        drawRect(fb, kToolbarWidth + 14, kTopBarHeight + 8, bW, 20, Color(70, 190, 220));
        drawText(fb, kToolbarWidth + 22, kTopBarHeight + 14, view.objective, 1, Color(160, 230, 255));
    }

    // Level 3 Polarity banners
    if (view.currentLevel == 3 && view.reversedPolarity) {
        fillRect(fb, 350, 600, 660, 52, Color(220, 40, 40));
        drawRect(fb, 350, 600, 660, 52, Color(255, 230, 80));
        drawText(fb, 370, 612, "POLARITY REVERSED! GENERATOR TURNS CCW (-)", 1, Color(255, 255, 255));
        drawText(fb, 370, 630, "INSERT 14T IDLER GEAR (SLOT 4) OR PRESS 'B' TO CROSS BELT FOR CW (+)", 1, Color(255, 240, 150));
    } else if (view.currentLevel == 3 && view.levelCompleted) {
        fillRect(fb, 350, 600, 660, 52, Color(20, 45, 30));
        drawRect(fb, 350, 600, 660, 52, Color(80, 230, 120));
        drawText(fb, 370, 612, "[POLARITY CORRECT: +14.4V DC!] - BULB 100% ILLUMINATED!", 1, Color(120, 255, 160));
        drawText(fb, 370, 630, "Generator is spinning Clockwise (+). Level 3 Cleared!", 1, Color(255, 255, 255));
    } else if (view.currentLevel == 3 && !view.levelCompleted && !view.powered) {
        fillRect(fb, 350, 600, 660, 52, Color(22, 32, 40));
        drawRect(fb, 350, 600, 660, 52, Color(70, 190, 220));
        drawText(fb, 370, 612, "[LEVEL 3: DIRECTION & POLARITY CHALLENGE]", 1, Color(160, 230, 255));
        drawText(fb, 370, 630, "Direct 2-gear mesh spins generator CCW (-). Insert 14T Idler or Cross Belt ('B')!", 1, Color(255, 230, 120));
    }

    // Level 2 Undervoltage banners
    if (view.currentLevel == 2 && view.voltage < 12.0f && view.powered) {
        fillRect(fb, 350, 600, 660, 52, Color(180, 70, 20));
        drawRect(fb, 350, 600, 660, 52, Color(255, 200, 50));
        drawText(fb, 370, 612, "UNDERVOLTAGE: 7.2V DC (NEED >= 12.0V) - BULB IS OFF!", 1, Color(255, 255, 255));
        drawText(fb, 370, 630, "MOTOR IS SLOW (400 RPM). SWAP TO 18T->12T OVERDRIVE RATIO!", 1, Color(255, 240, 150));
    } else if (view.currentLevel == 2 && view.levelCompleted) {
        fillRect(fb, 350, 600, 660, 52, Color(20, 45, 30));
        drawRect(fb, 350, 600, 660, 52, Color(80, 230, 120));
        drawText(fb, 370, 612, "[OVERDRIVE STEP-UP ONLINE: 14.4V DC!] - BULB 100% ILLUMINATED!", 1, Color(120, 255, 160));
        drawText(fb, 370, 630, "18T driving 12T stepped up RPM to overcome low motor speed. Level 2 Cleared!", 1, Color(255, 255, 255));
    } else if (view.currentLevel == 2 && !view.levelCompleted && !view.powered) {
        fillRect(fb, 350, 600, 660, 52, Color(22, 32, 40));
        drawRect(fb, 350, 600, 660, 52, Color(70, 190, 220));
        drawText(fb, 370, 612, "[LEVEL 2: OVERDRIVE STEP-UP CHALLENGE (TARGET >= 12.0V DC)]", 1, Color(160, 230, 255));
        drawText(fb, 370, 630, "Motor is half-speed (400 RPM). Mount 18T Gear on Motor -> 12T Gear on Pulley!", 1, Color(255, 230, 120));
    }

    // Level 1 Operational banner
    if (view.currentLevel == 1 && view.levelCompleted) {
        fillRect(fb, 350, 600, 660, 52, Color(20, 45, 30));
        drawRect(fb, 350, 600, 660, 52, Color(80, 230, 120));
        drawText(fb, 370, 612, "[ASSIGNMENT MACHINE OPERATIONAL!] 14.4V DC Generated!", 1, Color(120, 255, 160));
        drawText(fb, 370, 630, "12T -> 18T gear reduction driving belt generator. Level 1 Cleared!", 1, Color(255, 255, 255));
    } else if (view.currentLevel == 1 && !view.levelCompleted && !view.powered) {
        fillRect(fb, 350, 600, 660, 52, Color(22, 32, 40));
        drawRect(fb, 350, 600, 660, 52, Color(70, 190, 220));
        drawText(fb, 370, 612, "[LEVEL 1: BASELINE MACHINE (ASSIGNMENT SPECIFICATION)]", 1, Color(160, 230, 255));
        drawText(fb, 370, 630, "Motor (4.0 rad/s) -> 12T -> 18T -> 40px -> Belt -> 22px -> Bulb. Hit PLAY!", 1, Color(255, 230, 120));
    }

    if (view.selectedPart >= 0 && view.selectedPart < static_cast<int>(parts.size()) && parts[view.selectedPart].placed) {
        const PickCircle pc = pickCircle(parts[view.selectedPart]);
        const Color ringCol = view.isMeshed ? Color(100, 240, 140) : palette::glow;
        for (int k = 6; k <= 8; ++k) drawCircle(fb, pc.x, pc.y, pc.r + k, ringCol);
    }
}

void renderInterface(Framebuffer& fb, const SceneView& view) {
    drawToolbar(fb, view.hoveredSlot, view.currentLevel);
    drawTopBar(fb, view.hoveredButton, view.currentLevel, view.levelCompleted);

    // Bottom helper bar
    const int bY = fb.height() - 20;
    fillRect(fb, kToolbarWidth, bY - 4, fb.width() - kToolbarWidth, 24, Color(16, 22, 28));
    drawLine(fb, kToolbarWidth, bY - 4, fb.width() - 1, bY - 4, Color(35, 48, 58));
    drawText(fb, kToolbarWidth + 16, bY + 2,
             "CONTROLS: R-Click / DEL: Remove Part | Drag to Toolbox to Delete | 'B': Cross Belt | Space: Play | R: Reset | C: Clear | S: Solve",
             1, Color(140, 165, 180));

    if (view.hoveredSlot >= 0 && view.hoveredSlot < kSlotCount) {
        drawComponentInfoCard(fb, view.hoveredSlot, view.currentLevel, view.mouseX, view.mouseY);
    }
}

}  // namespace lumina
