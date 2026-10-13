#include "lumina/belt.h"

#include <cmath>

#include "lumina/line.h"

namespace lumina {

void drawBelt(Framebuffer& fb, float c1x, float c1y, int r1, float c2x, float c2y, int r2,
              int thickness, Color c) {
    const float dx = c2x - c1x, dy = c2y - c1y;
    const float d = std::sqrt(dx * dx + dy * dy);
    if (d <= std::abs(float(r1 - r2))) return;  // one circle inside the other

    // The tangent point on each circle lies along a normal n where
    // n . (c2 - c1) = r1 - r2, i.e. at angle phi +- acos((r1 - r2) / d).
    const float phi = std::atan2(dy, dx);
    const float off = std::acos((r1 - r2) / d);
    for (int side = -1; side <= 1; side += 2) {
        const float t = phi + side * off;
        const float nx = std::cos(t), ny = std::sin(t);
        drawThickLine(fb, static_cast<int>(std::lround(c1x + r1 * nx)), static_cast<int>(std::lround(c1y + r1 * ny)),
                      static_cast<int>(std::lround(c2x + r2 * nx)), static_cast<int>(std::lround(c2y + r2 * ny)),
                      thickness, c);
    }
}

}  // namespace lumina
