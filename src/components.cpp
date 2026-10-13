#include "lumina/components.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "lumina/circle.h"
#include "lumina/line.h"
#include "lumina/polygon.h"

namespace lumina {

namespace {
constexpr float kPi = 3.14159265358979f;
int ri(float v) { return static_cast<int>(std::lround(v)); }
}  // namespace

Component makeMotor(float x, float y, int radius) {
    Component c; c.type = ComponentType::Motor; c.x = x; c.y = y; c.radius = radius; return c;
}

Component makeGear(float x, float y, int teeth, float angle) {
    Component c;
    c.type = ComponentType::Gear; c.x = x; c.y = y; c.teeth = teeth; c.angle = angle;
    c.radius = ri(gearPitchRadius(teeth)) + kToothAddendum;
    return c;
}

Component makePulley(float x, float y, int radius) {
    Component c; c.type = ComponentType::Pulley; c.x = x; c.y = y; c.radius = radius; return c;
}

Component makeBulb(float x, float y, int radius, bool lit) {
    Component c; c.type = ComponentType::Lightbulb; c.x = x; c.y = y; c.radius = radius; c.lit = lit;
    return c;
}

float meshAngle(int teeth, float towardsAngle) {
    // A gap must face the neighbour: tooth centres sit half a pitch away.
    return towardsAngle + kPi / teeth;
}

void drawGear(Framebuffer& fb, const Component& c) {
    const int n = c.teeth;
    const float pitch = 2 * kPi / n;
    const float rPitch = gearPitchRadius(n);
    const float rTip = rPitch + kToothAddendum;
    const float rRoot = rPitch - kToothDedendum;

    std::vector<Point> poly;
    poly.reserve(static_cast<size_t>(n) * 4);
    for (int k = 0; k < n; ++k) {
        const float centre = c.angle + k * pitch;
        const float aRoot = 0.275f * pitch;  // tooth is wider at the root...
        const float aTip = 0.15f * pitch;    // ...than at the tip
        const float angles[4] = {centre - aRoot, centre - aTip, centre + aTip, centre + aRoot};
        const float radii[4] = {rRoot, rTip, rTip, rRoot};
        for (int i = 0; i < 4; ++i)
            poly.push_back({c.x + radii[i] * std::cos(angles[i]),
                            c.y + radii[i] * std::sin(angles[i])});
    }
    fillPolygon(fb, poly, palette::metalMid);
    drawPolygon(fb, poly, palette::outline);

    const int cx = ri(c.x), cy = ri(c.y);
    fillCircle(fb, cx, cy, ri(rRoot * 0.55f), palette::metalLight);   // hub
    drawCircle(fb, cx, cy, ri(rRoot * 0.55f), palette::outline);
    // Spokes make the rotation visible in CP2.
    for (int s = 0; s < 3; ++s) {
        const float a = c.angle + s * 2 * kPi / 3;
        drawThickLine(fb, cx, cy, ri(c.x + rRoot * 0.5f * std::cos(a)),
                      ri(c.y + rRoot * 0.5f * std::sin(a)), 3, palette::metalDark);
    }
    fillCircle(fb, cx, cy, 5, palette::outline);                       // axle hole
}

void drawPulley(Framebuffer& fb, const Component& c) {
    const int cx = ri(c.x), cy = ri(c.y), r = c.radius;
    fillCircle(fb, cx, cy, r, palette::metalMid);
    drawCircle(fb, cx, cy, r, palette::outline);
    drawCircle(fb, cx, cy, r - 1, palette::outline);
    fillCircle(fb, cx, cy, ri(r * 0.78f), palette::metalDark);          // the groove
    drawCircle(fb, cx, cy, ri(r * 0.78f), palette::outline);
    fillCircle(fb, cx, cy, ri(r * 0.55f), palette::metalLight);         // inner face
    drawCircle(fb, cx, cy, ri(r * 0.55f), palette::outline);
    // Marker spoke, so rotation can be seen.
    drawThickLine(fb, cx, cy, ri(c.x + r * 0.5f * std::cos(c.angle)),
                  ri(c.y + r * 0.5f * std::sin(c.angle)), 3, palette::metalDark);
    fillCircle(fb, cx, cy, 4, palette::outline);
}

void drawMotor(Framebuffer& fb, const Component& c) {
    // Body to the left of the centre, with the shaft at the centre point.
    const int r = c.radius;
    const int cx = ri(c.x), cy = ri(c.y);
    const int bodyL = cx - 2 * r, bodyW = 2 * r - r / 4, bodyH = ri(r * 1.5f);
    const int top = cy - bodyH / 2;

    fillRect(fb, bodyL, top, bodyW, bodyH, palette::motorBody);
    drawRect(fb, bodyL, top, bodyW, bodyH, palette::outline);
    fillRect(fb, bodyL, top, r / 3, bodyH, palette::motorCap);          // rear cap
    drawRect(fb, bodyL, top, r / 3, bodyH, palette::outline);
    for (int i = 1; i <= 3; ++i) {                                      // cooling fins
        const int fx = bodyL + r / 3 + i * (bodyW - r / 3) / 4;
        drawLine(fb, fx, top + 4, fx, top + bodyH - 5, palette::motorCap);
    }
    fillRect(fb, bodyL + bodyW, cy - bodyH / 4, r / 4, bodyH / 2, palette::motorCap);  // front plate
    drawRect(fb, bodyL + bodyW, cy - bodyH / 4, r / 4, bodyH / 2, palette::outline);
    fillCircle(fb, cx, cy, ri(r * 0.18f), palette::metalLight);        // shaft
    drawCircle(fb, cx, cy, ri(r * 0.18f), palette::outline);
}

void drawBulb(Framebuffer& fb, const Component& c) {
    const int cx = ri(c.x), cy = ri(c.y), r = c.radius;
    const Color glass = c.lit ? palette::bulbOn : palette::bulbOff;

    // Screw base: trapezoid under the glass, with thread lines.
    const float baseTop = c.y + r * 0.78f, baseBot = c.y + r * 1.55f;
    fillPolygon(fb, {{c.x - r * 0.45f, baseTop}, {c.x + r * 0.45f, baseTop},
                     {c.x + r * 0.35f, baseBot}, {c.x - r * 0.35f, baseBot}}, palette::metalMid);
    for (int i = 1; i <= 3; ++i) {
        const int ty = ri(baseTop + i * (baseBot - baseTop) / 4);
        drawLine(fb, cx - ri(r * 0.40f), ty, cx + ri(r * 0.40f), ty, palette::metalDark);
    }
    drawPolygon(fb, {{c.x - r * 0.45f, baseTop}, {c.x + r * 0.45f, baseTop},
                     {c.x + r * 0.35f, baseBot}, {c.x - r * 0.35f, baseBot}}, palette::outline);

    // Glass bulb.
    fillCircle(fb, cx, cy, r, glass);
    drawCircle(fb, cx, cy, r, palette::outline);
    // Filament.
    const Color fil = c.lit ? Color(255, 140, 30) : palette::metalDark;
    drawThickLine(fb, cx - r / 3, cy + r / 2, cx - r / 5, cy - r / 8, 2, fil);
    drawThickLine(fb, cx - r / 5, cy - r / 8, cx + r / 5, cy - r / 8, 2, fil);
    drawThickLine(fb, cx + r / 5, cy - r / 8, cx + r / 3, cy + r / 2, 2, fil);
    // Shine.
    fillCircle(fb, cx - r / 2, cy - r / 2, std::max(2, r / 9), c.lit ? Color(255, 250, 215) : palette::metalLight);
}

void drawComponent(Framebuffer& fb, const Component& c) {
    switch (c.type) {
        case ComponentType::Motor: drawMotor(fb, c); break;
        case ComponentType::Gear: drawGear(fb, c); break;
        case ComponentType::Pulley: drawPulley(fb, c); break;
        case ComponentType::Lightbulb: drawBulb(fb, c); break;
    }
}

}  // namespace lumina
