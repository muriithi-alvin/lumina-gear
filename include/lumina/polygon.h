// polygon.h - scanline polygon fill, used for gears, the motor body and UI.
#pragma once
#include <vector>

#include "lumina/framebuffer.h"

namespace lumina {

struct Point {
    float x = 0, y = 0;
};

// Fill a polygon (any simple shape, convex or not) using the even-odd rule.
void fillPolygon(Framebuffer& fb, const std::vector<Point>& pts, Color c);

// Outline of a polygon, drawn with Bresenham lines.
void drawPolygon(Framebuffer& fb, const std::vector<Point>& pts, Color c);

// Axis-aligned filled rectangle (used for buttons and the toolbar).
void fillRect(Framebuffer& fb, int x, int y, int w, int h, Color c);
void drawRect(Framebuffer& fb, int x, int y, int w, int h, Color c);

}  // namespace lumina
