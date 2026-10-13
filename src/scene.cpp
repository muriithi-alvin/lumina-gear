#include "lumina/scene.h"

#include <cmath>
#include <vector>

#include "lumina/belt.h"
#include "lumina/circle.h"
#include "lumina/components.h"
#include "lumina/effects.h"
#include "lumina/line.h"
#include "lumina/picking.h"
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

// The circle used for clicking a part and for its selection ring. The motor's
// centre is its shaft and its body sits to the left, so its circle moves left.
struct PickCircle { int x, y, r; };

PickCircle pickCircle(const Component& c) {
    if (c.type == ComponentType::Motor) return {ri(c.x) - c.radius, ri(c.y), c.radius};
    return {ri(c.x), ri(c.y), c.radius};
}
}  // namespace

std::vector<Component> sceneParts(bool powered) {
    // --- Layout of the machine (centre of the canvas, y = 330) ---
    const float midY = 330;
    std::vector<Component> parts(kPartCount);
    parts[kPartMotor] = makeMotor(210, midY, 46);
    parts[kPartGear1] = makeGear(340, midY, 12, 0);
    const float gear2X = parts[kPartGear1].x + gearPitchRadius(12) + gearPitchRadius(18);   // gears just touch
    parts[kPartGear2] = makeGear(gear2X, midY, 18, meshAngle(18, kPi));
    parts[kPartPulley] = makePulley(gear2X + 160, midY, 40);
    parts[kPartWheel] = makePulley(820, 440, 22);                                  // small belt wheel
    parts[kPartBulb] = makeBulb(820, 270, 44, powered);
    return parts;
}

int scenePartAt(int px, int py) {
    // Top-most first: the reverse of the drawing order in renderScene().
    const ScenePart order[] = {kPartBulb, kPartWheel, kPartPulley, kPartGear1, kPartGear2, kPartMotor};
    const std::vector<Component> parts = sceneParts(false);
    for (ScenePart p : order) {
        const PickCircle pc = pickCircle(parts[p]);
        if (pointInCircle(px, py, pc.x, pc.y, pc.r)) return p;
    }
    return -1;
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

void renderCanvas(Framebuffer& fb, const SceneView& view) {
    const bool powered = view.powered;
    fb.clear(palette::background);
    drawGrid(fb);

    const std::vector<Component> parts = sceneParts(powered);
    const Component& motor = parts[kPartMotor];
    const Component& gear1 = parts[kPartGear1];
    const Component& gear2 = parts[kPartGear2];
    const Component& pulley = parts[kPartPulley];
    const Component& wheel = parts[kPartWheel];
    const Component& bulb = parts[kPartBulb];

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

    // --- Selection ring (after the darkness, so it is never dimmed) ---
    if (view.selectedPart >= 0 && view.selectedPart < kPartCount) {
        const PickCircle pc = pickCircle(parts[view.selectedPart]);
        for (int k = 6; k <= 8; ++k) drawCircle(fb, pc.x, pc.y, pc.r + k, palette::glow);
    }
}

void renderInterface(Framebuffer& fb, const SceneView& view) {
    // --- Layer 4: user interface (never darkened) ---
    drawToolbar(fb, view.hoveredSlot);
    drawTopBar(fb, view.hoveredButton);
}

}  // namespace lumina
