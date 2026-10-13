// effects.h - alpha blending glow and the global darkness overlay.
#pragma once
#include <vector>

#include "lumina/framebuffer.h"

namespace lumina {

// A light source that "pierces" the darkness overlay.
struct Light {
    float x, y;
    float radius;      // how far the light reaches, in pixels
    float intensity;   // 0 (off) .. 1 (full)
};

// Soft glow: many semi-transparent concentric circles layered from the
// outside in, so the colour builds up towards the centre.
void drawGlow(Framebuffer& fb, int cx, int cy, int maxRadius, Color c, float intensity);

// Raster operation: darken the rectangle (x, y, w, h) with a black
// semi-transparent layer. Near each light the darkness is lifted and a warm
// additive colour is added, so the light seems to cut through the dark.
//   darkness: 0 (no overlay) .. 1 (pitch black)
void applyDarkness(Framebuffer& fb, int x, int y, int w, int h, float darkness,
                   const std::vector<Light>& lights, Color lightColor);

}  // namespace lumina
