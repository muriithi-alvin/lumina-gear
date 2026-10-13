#include "lumina/line.h"

#include <cmath>
#include <cstdlib>

namespace lumina {

void drawLine(Framebuffer& fb, int x0, int y0, int x1, int y1, Color c) {
    const int dx = std::abs(x1 - x0);
    const int dy = -std::abs(y1 - y0);
    const int sx = x0 < x1 ? 1 : -1;
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;  // error term; combines the x and y decisions

    while (true) {
        fb.setPixel(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        const int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }  // step in x
        if (e2 <= dx) { err += dx; y0 += sy; }  // step in y
    }
}

void drawThickLine(Framebuffer& fb, int x0, int y0, int x1, int y1, int thickness, Color c) {
    if (thickness <= 1) { drawLine(fb, x0, y0, x1, y1, c); return; }
    // Offset the line sideways (perpendicular to its direction).
    const float len = std::sqrt(float((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)));
    const float nx = len > 0 ? -(y1 - y0) / len : 0.0f;
    const float ny = len > 0 ? (x1 - x0) / len : 1.0f;
    for (int i = 0; i < thickness; ++i) {
        const float off = i - (thickness - 1) / 2.0f;
        const int ox = static_cast<int>(std::lround(nx * off));
        const int oy = static_cast<int>(std::lround(ny * off));
        drawLine(fb, x0 + ox, y0 + oy, x1 + ox, y1 + oy, c);
    }
}

}  // namespace lumina
