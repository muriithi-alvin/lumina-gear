// gl_renderer.h - Hardware-accelerated OpenGL renderer for Checkpoint 3 (CP3)
#pragma once

#include <vector>
#include "lumina/components.h"
#include "lumina/scene.h"

namespace lumina {

class GLRenderer {
public:
    static void init(int width, int height);
    static void reshape(int width, int height);
    static void render(const std::vector<Component>& parts, const SceneView& view, int width, int height);

private:
    static void drawCircle(float cx, float cy, float r, float red, float green, float blue, float alpha = 1.0f, bool filled = true, int segments = 48);
    static void drawRect(float x, float y, float w, float h, float red, float green, float blue, float alpha = 1.0f, bool filled = true);
    static void drawLine(float x1, float y1, float x2, float y2, float width, float red, float green, float blue, float alpha = 1.0f);
    static void drawGear(const Component& gear);
    static void drawPulley(const Component& pulley);
    static void drawMotor(const Component& motor);
    static void drawBulb(const Component& bulb, bool lit, float power);
    static void drawBelt(float x1, float y1, float r1, float x2, float y2, float r2, bool crossed);
    static void drawEngineValvetrain(const std::vector<Component>& parts, const SceneView& view);
    static void drawHardwareGlow(float cx, float cy, float radius, float intensity);
    static void drawTipCard(const std::vector<Component>& parts, const SceneView& view);
};

}  // namespace lumina
