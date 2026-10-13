#include "lumina/effects.h"

#include <algorithm>
#include <cmath>

#include "lumina/circle.h"

namespace lumina {

void drawGlow(Framebuffer& fb, int cx, int cy, int maxRadius, Color c, float intensity) {
    if (intensity <= 0 || maxRadius <= 0) return;
    const int rings = 16;
    for (int i = 0; i < rings; ++i) {
        // Largest ring first. Each one adds a little more colour.
        const float t = static_cast<float>(i) / (rings - 1);          // 0 outer .. 1 inner
        const int r = static_cast<int>(maxRadius * (1.0f - t * 0.92f));
        const float alpha = intensity * (0.035f + 0.05f * t);
        fillCircleBlend(fb, cx, cy, r, c, alpha);
    }
}

void applyDarkness(Framebuffer& fb, int x, int y, int w, int h, float darkness,
                   const std::vector<Light>& lights, Color lightColor) {
    const int x1 = std::min(fb.width(), x + w), y1 = std::min(fb.height(), y + h);
    for (int py = std::max(0, y); py < y1; ++py) {
        for (int px = std::max(0, x); px < x1; ++px) {
            // Total light reaching this pixel: smooth fall-off from each source.
            float light = 0;
            for (const Light& l : lights) {
                const float dx = px - l.x, dy = py - l.y;
                const float d2 = dx * dx + dy * dy;
                if (d2 >= l.radius * l.radius) continue;
                const float f = 1.0f - std::sqrt(d2) / l.radius;
                light += l.intensity * f * f;
            }
            light = std::min(1.0f, light);
            // 1) black overlay, weaker where there is light
            fb.blendPixel(px, py, Color(0, 0, 0), darkness * (1.0f - light));
            // 2) additive warm colour where there is light
            if (light > 0) fb.addPixel(px, py, lightColor, light * 0.35f);
        }
    }
}

}  // namespace lumina
