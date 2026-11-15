// gl_renderer.cpp - OpenGL hardware rendering implementation for CP3
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <cmath>
#include <cstdio>
#include <algorithm>

#include "lumina/gl_renderer.h"
#include "lumina/scene.h"
#include "lumina/simulation.h"

namespace lumina {

namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.0f * kPi;
}

void GLRenderer::init(int width, int height) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    reshape(width, height);
}

void GLRenderer::reshape(int width, int height) {
    if (height <= 0) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(kSceneWidth), static_cast<double>(kSceneHeight), 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLRenderer::drawCircle(float cx, float cy, float r, float red, float green, float blue, float alpha, bool filled, int segments) {
    glColor4f(red, green, blue, alpha);
    if (filled) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments; ++i) {
            const float a = i * kTwoPi / segments;
            glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
        }
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; ++i) {
            const float a = i * kTwoPi / segments;
            glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
        }
        glEnd();
    }
}

void GLRenderer::drawRect(float x, float y, float w, float h, float red, float green, float blue, float alpha, bool filled) {
    glColor4f(red, green, blue, alpha);
    if (filled) {
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }
}

void GLRenderer::drawLine(float x1, float y1, float x2, float y2, float width, float red, float green, float blue, float alpha) {
    glColor4f(red, green, blue, alpha);
    glLineWidth(width);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    glLineWidth(1.0f);
}

void GLRenderer::drawGear(const Component& gear) {
    const float cx = gear.x;
    const float cy = gear.y;
    const float r = static_cast<float>(gear.radius);
    const int teeth = std::max(gear.teeth, 6);

    // 1. Gear hub body
    drawCircle(cx, cy, r - 5.0f, 0.32f, 0.38f, 0.44f, 1.0f, true, 36);
    drawCircle(cx, cy, r - 5.0f, 0.15f, 0.20f, 0.25f, 1.0f, false, 36);

    // 2. Hardware gear teeth
    const float toothAngle = kTwoPi / teeth;
    const float halfTooth = toothAngle * 0.28f;
    const float tipR = r + 7.0f;
    const float rootR = r - 5.0f;

    glColor4f(0.40f, 0.48f, 0.55f, 1.0f);
    glBegin(GL_QUADS);
    for (int i = 0; i < teeth; ++i) {
        const float midA = gear.angle + i * toothAngle;
        const float a1 = midA - halfTooth * 1.3f;
        const float a2 = midA - halfTooth * 0.7f;
        const float a3 = midA + halfTooth * 0.7f;
        const float a4 = midA + halfTooth * 1.3f;

        glVertex2f(cx + rootR * std::cos(a1), cy + rootR * std::sin(a1));
        glVertex2f(cx + tipR  * std::cos(a2), cy + tipR  * std::sin(a2));
        glVertex2f(cx + tipR  * std::cos(a3), cy + tipR  * std::sin(a3));
        glVertex2f(cx + rootR * std::cos(a4), cy + rootR * std::sin(a4));
    }
    glEnd();

    // 3. Central axle and spoke cutouts
    drawCircle(cx, cy, 14.0f, 0.18f, 0.22f, 0.28f, 1.0f, true, 24);
    drawCircle(cx, cy, 6.0f, 0.55f, 0.65f, 0.75f, 1.0f, true, 16);

    // Spokes
    for (int s = 0; s < 3; ++s) {
        const float a = gear.angle + s * kTwoPi / 3.0f;
        drawLine(cx + 8.0f * std::cos(a), cy + 8.0f * std::sin(a),
                 cx + (r - 10.0f) * std::cos(a), cy + (r - 10.0f) * std::sin(a),
                 2.5f, 0.15f, 0.20f, 0.25f);
    }
}

void GLRenderer::drawPulley(const Component& pulley) {
    const float cx = pulley.x;
    const float cy = pulley.y;
    const float r = static_cast<float>(pulley.radius);

    drawCircle(cx, cy, r, 0.28f, 0.34f, 0.40f, 1.0f, true, 36);
    drawCircle(cx, cy, r, 0.15f, 0.20f, 0.25f, 1.0f, false, 36);
    drawCircle(cx, cy, r * 0.70f, 0.22f, 0.26f, 0.32f, 1.0f, true, 24);
    drawCircle(cx, cy, 6.0f, 0.65f, 0.75f, 0.85f, 1.0f, true, 16);
}

void GLRenderer::drawMotor(const Component& motor) {
    const float cx = motor.x;
    const float cy = motor.y;
    const float r = static_cast<float>(motor.radius);

    // Motor casing
    drawRect(cx - r * 1.5f, cy - r, r * 2.2f, r * 2.0f, 0.18f, 0.25f, 0.35f, 1.0f, true);
    drawRect(cx - r * 1.5f, cy - r, r * 2.2f, r * 2.0f, 0.35f, 0.45f, 0.55f, 1.0f, false);

    // Shaft collar
    drawRect(cx + r * 0.7f, cy - 6.0f, 18.0f, 12.0f, 0.50f, 0.58f, 0.65f, 1.0f, true);
    drawCircle(cx + r * 0.7f + 9.0f, cy, 5.0f, 0.20f, 0.25f, 0.30f, 1.0f, true, 16);
}

void GLRenderer::drawBulb(const Component& bulb, bool lit, float power) {
    const float cx = bulb.x;
    const float cy = bulb.y;
    const float r = static_cast<float>(bulb.radius);

    // Screw base
    drawRect(cx - 10.0f, cy + r - 2.0f, 20.0f, 16.0f, 0.45f, 0.48f, 0.52f, 1.0f, true);
    drawRect(cx - 6.0f, cy + r + 14.0f, 12.0f, 6.0f, 0.25f, 0.28f, 0.30f, 1.0f, true);

    // Glass bulb globe
    if (lit) {
        const float intensity = std::clamp(power, 0.0f, 1.0f);
        drawCircle(cx, cy, r, 1.0f, 0.95f, 0.60f + 0.35f * intensity, 0.92f, true, 36);
        drawCircle(cx, cy, r, 1.0f, 0.85f, 0.30f, 1.0f, false, 36);

        // Bright filament
        drawLine(cx - 8.0f, cy + 4.0f, cx, cy - 10.0f, 2.5f, 1.0f, 1.0f, 0.9f);
        drawLine(cx, cy - 10.0f, cx + 8.0f, cy + 4.0f, 2.5f, 1.0f, 1.0f, 0.9f);
    } else {
        drawCircle(cx, cy, r, 0.25f, 0.30f, 0.35f, 0.65f, true, 36);
        drawCircle(cx, cy, r, 0.35f, 0.42f, 0.50f, 1.0f, false, 36);
        drawLine(cx - 8.0f, cy + 4.0f, cx, cy - 10.0f, 1.5f, 0.45f, 0.45f, 0.45f);
        drawLine(cx, cy - 10.0f, cx + 8.0f, cy + 4.0f, 1.5f, 0.45f, 0.45f, 0.45f);
    }
}

void GLRenderer::drawBelt(float x1, float y1, float r1, float x2, float y2, float r2, bool crossed) {
    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float dist = std::hypot(dx, dy);
    if (dist <= 1.0f) return;

    const float angle = std::atan2(dy, dx);
    const float perpA = angle + kPi * 0.5f;
    const float perpB = crossed ? (angle - kPi * 0.5f) : (angle + kPi * 0.5f);

    const float p1x = x1 + r1 * std::cos(perpA);
    const float p1y = y1 + r1 * std::sin(perpA);
    const float p2x = x2 + r2 * std::cos(perpB);
    const float p2y = y2 + r2 * std::sin(perpB);

    const float q1x = x1 - r1 * std::cos(perpA);
    const float q1y = y1 - r1 * std::sin(perpA);
    const float q2x = x2 - r2 * std::cos(perpB);
    const float q2y = y2 - r2 * std::sin(perpB);

    // Thick belt lines using OpenGL
    drawLine(p1x, p1y, p2x, p2y, 4.0f, 0.55f, 0.40f, 0.28f);
    drawLine(q1x, q1y, q2x, q2y, 4.0f, 0.55f, 0.40f, 0.28f);
}

void GLRenderer::drawHardwareGlow(float cx, float cy, float radius, float intensity) {
    // Hardware-accelerated multi-fan smooth additive radial illumination (CP3 Key Technique)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for pure light glow

    const int rings = 18;
    for (int r = rings; r >= 1; --r) {
        const float t = static_cast<float>(r) / rings;
        const float rad = radius * t;
        const float alpha = (1.0f - t * t) * 0.12f * std::clamp(intensity, 0.0f, 1.0f);

        glColor4f(1.0f, 0.88f, 0.45f, alpha);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= 36; ++i) {
            const float a = i * kTwoPi / 36.0f;
            glVertex2f(cx + rad * std::cos(a), cy + rad * std::sin(a));
        }
        glEnd();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GLRenderer::drawEngineValvetrain(const std::vector<Component>& parts, const SceneView& view) {
    if (parts.size() < kEnginePartCount) return;

    const Component& crank = parts[kEngineCrank];
    const Component& cam = parts[kEngineCam];
    const Component& tens = parts[kEngineTensioner];
    const Component& piston = parts[kEnginePiston];
    const Component& alt = parts[kEngineAlternator];

    // Engine block casing
    drawRect(560.0f, 180.0f, 220.0f, 320.0f, 0.14f, 0.18f, 0.22f, 1.0f, true);
    drawRect(560.0f, 180.0f, 220.0f, 320.0f, 0.25f, 0.32f, 0.38f, 1.0f, false);

    // Overhead camshaft
    drawLine(cam.x, cam.y, 760.0f, cam.y, 8.0f, 0.45f, 0.52f, 0.58f);

    // Reciprocating piston (Harmonic motion)
    const float py = view.pistonY;
    drawRect(piston.x - 45.0f, py - 20.0f, 90.0f, 55.0f, 0.38f, 0.45f, 0.52f, 1.0f, true);
    drawRect(piston.x - 45.0f, py - 20.0f, 90.0f, 55.0f, 0.60f, 0.70f, 0.80f, 1.0f, false);

    // Connecting rod
    const float crankPinX = crank.x + 38.0f * std::cos(view.crankAngle);
    const float crankPinY = crank.y + 38.0f * std::sin(view.crankAngle);
    drawLine(crankPinX, crankPinY, piston.x, py + 10.0f, 7.0f, 0.48f, 0.55f, 0.62f);

    // Intake and exhaust poppet valves
    const float inLift = view.intakeLift;
    const float exLift = view.exhaustLift;
    drawLine(600.0f, 195.0f + inLift, 600.0f, 260.0f + inLift, 4.0f, 0.40f, 0.70f, 0.95f);
    drawLine(720.0f, 195.0f + exLift, 720.0f, 260.0f + exLift, 4.0f, 0.95f, 0.55f, 0.30f);

    // Belts & sprockets
    drawBelt(crank.x, crank.y, static_cast<float>(crank.radius),
             cam.x, cam.y, static_cast<float>(cam.radius), false);

    if (tens.placed) drawPulley(tens);
    if (crank.placed) drawGear(crank);
    if (cam.placed) drawGear(cam);
    if (alt.placed) {
        drawCircle(alt.x, alt.y, static_cast<float>(alt.radius), 0.35f, 0.45f, 0.55f, 1.0f, true);
        drawBelt(crank.x, crank.y, static_cast<float>(crank.radius),
                 alt.x, alt.y, static_cast<float>(alt.radius), false);
    }
}

void GLRenderer::drawTipCard(const std::vector<Component>& parts, const SceneView& view) {
    int cardX, cardY, cardW, cardH;
    getTipCardBounds(parts, view, cardX, cardY, cardW, cardH);

    float bgR = 0.08f, bgG = 0.12f, bgB = 0.16f;
    float bR = 0.28f, bG = 0.75f, bB = 0.85f;

    if (view.levelCompleted) {
        bgR = 0.08f; bgG = 0.18f; bgB = 0.12f;
        bR = 0.30f; bG = 0.90f; bB = 0.48f;
    } else if (view.collision) {
        bgR = 0.25f; bgG = 0.08f; bgB = 0.08f;
        bR = 0.95f; bG = 0.35f; bB = 0.35f;
    }

    drawRect(static_cast<float>(cardX), static_cast<float>(cardY),
             static_cast<float>(cardW), static_cast<float>(cardH),
             bgR, bgG, bgB, 0.92f, true);
    drawRect(static_cast<float>(cardX), static_cast<float>(cardY),
             static_cast<float>(cardW), static_cast<float>(cardH),
             bR, bG, bB, 1.0f, false);
}

void GLRenderer::render(const std::vector<Component>& parts, const SceneView& view, int width, int height) {
    reshape(width, height);

    glClearColor(0.07f, 0.09f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Subtle background grid
    glColor4f(0.12f, 0.15f, 0.18f, 0.4f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int x = 0; x < kSceneWidth; x += 40) {
        glVertex2f(static_cast<float>(x), 0.0f);
        glVertex2f(static_cast<float>(x), static_cast<float>(kSceneHeight));
    }
    for (int y = 0; y < kSceneHeight; y += 40) {
        glVertex2f(0.0f, static_cast<float>(y));
        glVertex2f(static_cast<float>(kSceneWidth), static_cast<float>(y));
    }
    glEnd();

    if (view.currentLevel >= 4) {
        drawEngineValvetrain(parts, view);
    } else {
        if (parts.size() >= kPartCount) {
            const Component& motor = parts[kPartMotor];
            const Component& gear1 = parts[kPartGear1];
            const Component& gear2 = parts[kPartGear2];
            const Component& pulley = parts[kPartPulley];
            const Component& wheel = parts[kPartWheel];
            const Component& bulb = parts[kPartBulb];

            // Drive shafts
            if (motor.placed && gear1.placed) drawLine(motor.x, motor.y, gear1.x, gear1.y, 6.0f, 0.38f, 0.44f, 0.50f);
            if (gear2.placed && pulley.placed) drawLine(gear2.x, gear2.y, pulley.x, pulley.y, 6.0f, 0.38f, 0.44f, 0.50f);

            // Belts
            if (pulley.placed && wheel.placed) {
                drawBelt(pulley.x, pulley.y, static_cast<float>(pulley.radius),
                         wheel.x, wheel.y, static_cast<float>(wheel.radius), view.crossedBelt);
            }

            // Components
            if (motor.placed) drawMotor(motor);
            if (gear2.placed) drawGear(gear2);
            if (gear1.placed) drawGear(gear1);
            if (pulley.placed) drawPulley(pulley);
            if (wheel.placed) drawPulley(wheel);
            if (bulb.placed) drawBulb(bulb, view.powered, view.powerLevel);

            // Hardware radial lighting
            if (bulb.placed && view.powered) {
                drawHardwareGlow(bulb.x, bulb.y, 520.0f, view.powerLevel);
            }
        }
    }

    drawTipCard(parts, view);
    glFlush();
}

}  // namespace lumina
