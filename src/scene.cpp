#include "lumina/scene.h"

#include <cmath>
#include <vector>

#include "lumina/belt.h"
#include "lumina/circle.h"
#include "lumina/components.h"
#include "lumina/effects.h"
#include "lumina/line.h"
#include "lumina/ui.h"

namespace lumina {

namespace {
constexpr float kPi = 3.14159265358979f;
int ri(float v) { return static_cast<int>(std::lround(v)); }

// Faint dot grid on the canvas, so it feels like a workbench.
void drawGrid(Framebuffer& fb) {
    const Color dot(58, 74, 83);
    for (int y = kTopBarHeight + 20; y < fb.height(); y += 40)
        for (int x = kToolbarWidth + 20; x < fb.width(); x += 40) fb.setPixel(x, y, dot);
}

void drawShaft(Framebuffer& fb, const Component& a, const Component& b) {
    drawThickLine(fb, ri(a.x), ri(a.y), ri(b.x), ri(b.y), 8, palette::metalDark);
    drawThickLine(fb, ri(a.x), ri(a.y) - 2, ri(b.x), ri(b.y) - 2, 2, palette::metalLight);
}
}  // namespace

void renderScene(Framebuffer& fb, bool powered) {
    fb.clear(palette::background);
    drawGrid(fb);

    // --- Layout of the machine (centre of the canvas, y = 330) ---
    const float midY = 330;
    Component motor = makeMotor(210, midY, 46);
    Component gear1 = makeGear(340, midY, 12, 0);
    const float gear2X = gear1.x + gearPitchRadius(12) + gearPitchRadius(18);   // gears just touch
    Component gear2 = makeGear(gear2X, midY, 18, meshAngle(18, kPi));
    Component pulley = makePulley(gear2X + 160, midY, 40);
    Component wheel = makePulley(820, 440, 22);                                  // small belt wheel
    Component bulb = makeBulb(820, 270, 44, powered);

    // --- Layer 1: things that sit behind the parts ---
    drawShaft(fb, motor, gear1);       // motor shaft drives gear 1
    drawShaft(fb, gear2, pulley);      // gear 2 turns the pulley
    drawBelt(fb, pulley.x, pulley.y, pulley.radius, wheel.x, wheel.y, wheel.radius, 4, palette::belt);
    drawThickLine(fb, ri(wheel.x), ri(wheel.y), ri(bulb.x), ri(bulb.y + bulb.radius * 1.3f), 5,
                  palette::metalDark);                                           // wheel powers the bulb

    // --- Layer 2: the parts ---
    drawMotor(fb, motor);
    drawGear(fb, gear2);
    drawGear(fb, gear1);
    drawPulley(fb, pulley);
    drawPulley(fb, wheel);

    // --- Layer 3: light ---
    if (powered) drawGlow(fb, ri(bulb.x), ri(bulb.y), 130, palette::glow, 1.0f);
    drawBulb(fb, bulb);

    std::vector<Light> lights;
    if (powered) lights.push_back({bulb.x, bulb.y, 560.0f, 1.0f});
    applyDarkness(fb, kToolbarWidth, kTopBarHeight, fb.width() - kToolbarWidth,
                  fb.height() - kTopBarHeight, powered ? 0.40f : 0.60f, lights, palette::glow);

    // --- Layer 4: user interface (never darkened) ---
    drawToolbar(fb);
    drawTopBar(fb);
}

}  // namespace lumina
