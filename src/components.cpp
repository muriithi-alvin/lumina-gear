#include "lumina/components.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "lumina/circle.h"
#include "lumina/effects.h"
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

Component makeCamLobe(float x, float y, int radius, float angle) {
    Component c; c.type = ComponentType::CamLobe; c.x = x; c.y = y; c.radius = radius; c.angle = angle;
    return c;
}

Component makeValve(float x, float y, int radius) {
    Component c; c.type = ComponentType::Valve; c.x = x; c.y = y; c.radius = radius;
    return c;
}

Component makePiston(float x, float y, int radius) {
    Component c; c.type = ComponentType::Piston; c.x = x; c.y = y; c.radius = radius;
    return c;
}

Component makeAlternator(float x, float y, int radius) {
    Component c; c.type = ComponentType::Alternator; c.x = x; c.y = y; c.radius = radius;
    return c;
}

float meshAngle(int teeth, float towardsAngle) {
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
        const float aRoot = 0.275f * pitch;
        const float aTip = 0.15f * pitch;
        const float angles[4] = {centre - aRoot, centre - aTip, centre + aTip, centre + aRoot};
        const float radii[4] = {rRoot, rTip, rTip, rRoot};
        for (int i = 0; i < 4; ++i)
            poly.push_back({c.x + radii[i] * std::cos(angles[i]),
                            c.y + radii[i] * std::sin(angles[i])});
    }
    fillPolygon(fb, poly, palette::metalMid);
    drawPolygon(fb, poly, palette::outline);

    const int cx = ri(c.x), cy = ri(c.y);
    fillCircle(fb, cx, cy, ri(rRoot * 0.55f), palette::metalLight);
    drawCircle(fb, cx, cy, ri(rRoot * 0.55f), palette::outline);

    // Spokes make the rotation visible in CP2.
    for (int s = 0; s < 3; ++s) {
        const float a = c.angle + s * 2 * kPi / 3;
        drawThickLine(fb, cx, cy, ri(c.x + rRoot * 0.5f * std::cos(a)),
                      ri(c.y + rRoot * 0.5f * std::sin(a)), 3, palette::metalDark);
    }
    fillCircle(fb, cx, cy, 5, palette::outline);

    // Timing mark dot on outer tooth (for camshaft timing alignment)
    const int tx = ri(c.x + (rTip - 2) * std::cos(c.angle));
    const int ty = ri(c.y + (rTip - 2) * std::sin(c.angle));
    fillCircle(fb, tx, ty, 3, Color(255, 230, 80));
    drawCircle(fb, tx, ty, 3, palette::outline);
}

void drawPulley(Framebuffer& fb, const Component& c) {
    const int cx = ri(c.x), cy = ri(c.y), r = c.radius;
    fillCircle(fb, cx, cy, r, palette::metalMid);
    drawCircle(fb, cx, cy, r, palette::outline);
    drawCircle(fb, cx, cy, r - 1, palette::outline);
    fillCircle(fb, cx, cy, ri(r * 0.78f), palette::metalDark);
    drawCircle(fb, cx, cy, ri(r * 0.78f), palette::outline);
    fillCircle(fb, cx, cy, ri(r * 0.55f), palette::metalLight);
    drawCircle(fb, cx, cy, ri(r * 0.55f), palette::outline);
    // Marker spoke
    drawThickLine(fb, cx, cy, ri(c.x + r * 0.5f * std::cos(c.angle)),
                  ri(c.y + r * 0.5f * std::sin(c.angle)), 3, palette::metalDark);
    fillCircle(fb, cx, cy, 4, palette::outline);
}

void drawMotor(Framebuffer& fb, const Component& c) {
    const int r = c.radius;
    const int cx = ri(c.x), cy = ri(c.y);
    const int bodyL = cx - 2 * r, bodyW = 2 * r - r / 4, bodyH = ri(r * 1.5f);
    const int top = cy - bodyH / 2;

    fillRect(fb, bodyL, top, bodyW, bodyH, palette::motorBody);
    drawRect(fb, bodyL, top, bodyW, bodyH, palette::outline);
    fillRect(fb, bodyL, top, r / 3, bodyH, palette::motorCap);
    drawRect(fb, bodyL, top, r / 3, bodyH, palette::outline);
    for (int i = 1; i <= 3; ++i) {
        const int fx = bodyL + r / 3 + i * (bodyW - r / 3) / 4;
        drawLine(fb, fx, top + 4, fx, top + bodyH - 5, palette::motorCap);
    }
    fillRect(fb, bodyL + bodyW, cy - bodyH / 4, r / 4, bodyH / 2, palette::motorCap);
    drawRect(fb, bodyL + bodyW, cy - bodyH / 4, r / 4, bodyH / 2, palette::outline);
    fillCircle(fb, cx, cy, ri(r * 0.18f), palette::metalLight);
    drawCircle(fb, cx, cy, ri(r * 0.18f), palette::outline);

    // Marker on shaft
    const float sRad = r * 0.18f;
    drawLine(fb, cx, cy, ri(cx + sRad * std::cos(c.angle)), ri(cy + sRad * std::sin(c.angle)), palette::outline);
}

void drawBulb(Framebuffer& fb, const Component& c) {
    const int cx = ri(c.x), cy = ri(c.y), r = c.radius;
    const Color glass = c.lit ? palette::bulbOn : palette::bulbOff;

    const float baseTop = c.y + r * 0.78f, baseBot = c.y + r * 1.55f;
    fillPolygon(fb, {{c.x - r * 0.45f, baseTop}, {c.x + r * 0.45f, baseTop},
                     {c.x + r * 0.35f, baseBot}, {c.x - r * 0.35f, baseBot}}, palette::metalMid);
    for (int i = 1; i <= 3; ++i) {
        const int ty = ri(baseTop + i * (baseBot - baseTop) / 4);
        drawLine(fb, cx - ri(r * 0.40f), ty, cx + ri(r * 0.40f), ty, palette::metalDark);
    }
    drawPolygon(fb, {{c.x - r * 0.45f, baseTop}, {c.x + r * 0.45f, baseTop},
                     {c.x + r * 0.35f, baseBot}, {c.x - r * 0.35f, baseBot}}, palette::outline);

    fillCircle(fb, cx, cy, r, glass);
    drawCircle(fb, cx, cy, r, palette::outline);

    const Color fil = c.lit ? Color(255, 140, 30) : palette::metalDark;
    drawThickLine(fb, cx - r / 3, cy + r / 2, cx - r / 5, cy - r / 8, 2, fil);
    drawThickLine(fb, cx - r / 5, cy - r / 8, cx + r / 5, cy - r / 8, 2, fil);
    drawThickLine(fb, cx + r / 5, cy - r / 8, cx + r / 3, cy + r / 2, 2, fil);
    fillCircle(fb, cx - r / 2, cy - r / 2, std::max(2, r / 9), c.lit ? Color(255, 250, 215) : palette::metalLight);
}

void drawCamLobe(Framebuffer& fb, float cx, float cy, float radius, float angle) {
    const int N = 28;
    std::vector<Point> lobePoly;
    lobePoly.reserve(N);
    for (int i = 0; i < N; ++i) {
        const float a = i * 2.0f * kPi / N;
        const float cosDiff = std::cos(a - angle);
        const float lift = (cosDiff > 0.0f) ? (cosDiff * cosDiff * 0.72f) : 0.0f;
        const float r = radius * (1.0f + lift);
        lobePoly.push_back({cx + r * std::cos(a), cy + r * std::sin(a)});
    }
    fillPolygon(fb, lobePoly, palette::metalMid);
    drawPolygon(fb, lobePoly, palette::outline);

    // Camshaft axle hole
    fillCircle(fb, ri(cx), ri(cy), ri(radius * 0.35f), palette::metalLight);
    drawCircle(fb, ri(cx), ri(cy), ri(radius * 0.35f), palette::outline);
    // Keyway pin
    fillCircle(fb, ri(cx), ri(cy), 3, palette::metalDark);
}

void drawValve(Framebuffer& fb, float cx, float topY, float lift, float stemLength, Color springColor) {
    const int icx = ri(cx);
    const int retainerY = ri(topY + lift);
    const int headY = ri(topY + lift + stemLength);

    // Top spring retainer disc
    fillRect(fb, icx - 12, retainerY - 4, 24, 5, palette::metalDark);
    drawRect(fb, icx - 12, retainerY - 4, 24, 5, palette::outline);

    // Valve guide in cylinder head
    const int guideTop = ri(topY + 18);
    const int guideBot = ri(topY + stemLength - 16);
    fillRect(fb, icx - 6, guideTop, 12, guideBot - guideTop, palette::metalLight);
    drawRect(fb, icx - 6, guideTop, 12, guideBot - guideTop, palette::outline);

    // Valve stem
    drawThickLine(fb, icx, retainerY, icx, headY, 4, palette::metalLight);

    // Coiled Return Spring
    const int springSeatY = guideTop;
    const int numCoils = 6;
    if (springSeatY > retainerY + 4) {
        const float coilH = static_cast<float>(springSeatY - retainerY) / numCoils;
        for (int i = 0; i < numCoils; ++i) {
            const int y0 = ri(retainerY + i * coilH);
            const int y1 = ri(retainerY + (i + 1) * coilH);
            const int x0 = (i % 2 == 0) ? -9 : 9;
            const int x1 = (i % 2 == 0) ? 9 : -9;
            drawLine(fb, icx + x0, y0, icx + x1, y1, springColor);
            drawLine(fb, icx + x0, y0 + 1, icx + x1, y1 + 1, springColor);
        }
    }

    // Beveled poppet valve head
    const float wHead = 20.0f;
    const float hHead = 7.0f;
    std::vector<Point> headPoly = {
        {cx - wHead, headY + hHead},
        {cx + wHead, headY + hHead},
        {cx + wHead * 0.7f, static_cast<float>(headY)},
        {cx - wHead * 0.7f, static_cast<float>(headY)}
    };
    fillPolygon(fb, headPoly, palette::metalMid);
    drawPolygon(fb, headPoly, palette::outline);
}

void drawPistonAndCylinder(Framebuffer& fb, float cx, float cy, float width, float height,
                           float pistonY, float rodAngle, bool combustion) {
    const int icx = ri(cx);
    const int leftX = ri(cx - width * 0.5f);
    const int rightX = ri(cx + width * 0.5f);
    const int topY = ri(cy);
    const int botY = ri(cy + height);

    // Cylinder Combustion Chamber Interior (Dark bore)
    fillRect(fb, leftX + 1, topY, ri(width) - 2, ri(height), Color(30, 36, 42));

    // Combustion flash / explosion if firing
    if (combustion) {
        const int fireH = std::max(10, ri(pistonY - topY));
        fillRect(fb, leftX + 2, topY, ri(width) - 4, fireH, Color(255, 140, 20));
        drawGlow(fb, icx, topY + fireH / 2, ri(width * 0.6f), Color(255, 230, 80), 0.9f);
    }

    // Cylinder walls (Cast iron liner)
    drawThickLine(fb, leftX, topY, leftX, botY, 6, palette::metalMid);
    drawThickLine(fb, rightX, topY, rightX, botY, 6, palette::metalMid);
    drawLine(fb, leftX - 3, topY, leftX - 3, botY, palette::outline);
    drawLine(fb, rightX + 3, topY, rightX + 3, botY, palette::outline);

    // Piston Crown & Skirt
    const int pW = ri(width) - 6;
    const int pH = 44;
    const int pTop = ri(pistonY);
    const int pLeft = icx - pW / 2;

    fillRect(fb, pLeft, pTop, pW, pH, palette::metalLight);
    drawRect(fb, pLeft, pTop, pW, pH, palette::outline);

    // Compression Ring Grooves
    drawLine(fb, pLeft, pTop + 8, pLeft + pW, pTop + 8, palette::metalDark);
    drawLine(fb, pLeft, pTop + 14, pLeft + pW, pTop + 14, palette::metalDark);
    drawLine(fb, pLeft, pTop + 20, pLeft + pW, pTop + 20, palette::metalDark);

    // Wrist Pin (gudgeon pin)
    fillCircle(fb, icx, pTop + 26, 7, palette::metalMid);
    drawCircle(fb, icx, pTop + 26, 7, palette::outline);

    // Connecting Rod
    const int rodLen = 95;
    const int crankPinX = ri(icx + rodLen * std::sin(rodAngle));
    const int crankPinY = ri(pTop + 26 + rodLen * std::cos(rodAngle));
    drawThickLine(fb, icx, pTop + 26, crankPinX, crankPinY, 8, palette::metalMid);
    drawThickLine(fb, icx, pTop + 26, crankPinX, crankPinY, 2, palette::metalLight);
}

void drawAlternator(Framebuffer& fb, const Component& c, float rotationAngle, bool isGenerating) {
    const int cx = ri(c.x), cy = ri(c.y), r = c.radius;

    // Mounting bracket tabs
    fillRect(fb, cx - r - 6, cy - 8, 8, 16, palette::metalDark);
    drawRect(fb, cx - r - 6, cy - 8, 8, 16, palette::outline);
    fillRect(fb, cx + r - 2, cy - 8, 8, 16, palette::metalDark);
    drawRect(fb, cx + r - 2, cy - 8, 8, 16, palette::outline);

    // Main stator housing (cast aluminum)
    fillCircle(fb, cx, cy, r, palette::metalMid);
    drawCircle(fb, cx, cy, r, palette::outline);

    // Internal copper windings ring
    fillCircle(fb, cx, cy, ri(r * 0.78f), palette::copper);
    drawCircle(fb, cx, cy, ri(r * 0.78f), palette::outline);

    // Stator pole teeth / ventilation louvres
    const int numPoles = 8;
    for (int i = 0; i < numPoles; ++i) {
        const float a = i * 2.0f * kPi / numPoles;
        const int x0 = ri(cx + r * 0.45f * std::cos(a));
        const int y0 = ri(cy + r * 0.45f * std::sin(a));
        const int x1 = ri(cx + r * 0.75f * std::cos(a));
        const int y1 = ri(cy + r * 0.75f * std::sin(a));
        drawLine(fb, x0, y0, x1, y1, palette::metalDark);
    }

    // Rotor pulley & spinning cooling fan blades
    fillCircle(fb, cx, cy, ri(r * 0.44f), palette::metalLight);
    drawCircle(fb, cx, cy, ri(r * 0.44f), palette::outline);

    // Fan blades rotating with rotationAngle
    const int fanBlades = 6;
    for (int i = 0; i < fanBlades; ++i) {
        const float a = rotationAngle + i * 2.0f * kPi / fanBlades;
        const int fx = ri(cx + r * 0.40f * std::cos(a));
        const int fy = ri(cy + r * 0.40f * std::sin(a));
        drawLine(fb, cx, cy, fx, fy, palette::metalDark);
    }

    // Center axle shaft
    fillCircle(fb, cx, cy, 4, palette::metalDark);

    // Output terminal posts (+ and -)
    fillCircle(fb, cx - 12, cy - r - 4, 4, Color(220, 50, 40));
    drawCircle(fb, cx - 12, cy - r - 4, 4, palette::outline);
    fillCircle(fb, cx + 12, cy - r - 4, 4, Color(30, 35, 40));
    drawCircle(fb, cx + 12, cy - r - 4, 4, palette::outline);

    // Active power generation electrical aura
    if (isGenerating) {
        drawCircle(fb, cx, cy, r + 2, Color(100, 200, 255));
        drawCircle(fb, cx, cy, r + 3, Color(255, 230, 100));
    }
}

void drawComponent(Framebuffer& fb, const Component& c) {
    switch (c.type) {
        case ComponentType::Motor: drawMotor(fb, c); break;
        case ComponentType::Gear: drawGear(fb, c); break;
        case ComponentType::Pulley: drawPulley(fb, c); break;
        case ComponentType::Lightbulb: drawBulb(fb, c); break;
        case ComponentType::CamLobe: drawCamLobe(fb, c.x, c.y, static_cast<float>(c.radius), c.angle); break;
        case ComponentType::Valve: drawValve(fb, c.x, c.y, 0.0f, 60.0f, palette::bronze); break;
        case ComponentType::Piston: drawPistonAndCylinder(fb, c.x, c.y, 90.0f, 150.0f, c.y + 40.0f, 0.0f, false); break;
        case ComponentType::Alternator: drawAlternator(fb, c, c.angle, c.lit); break;
    }
}

}  // namespace lumina
