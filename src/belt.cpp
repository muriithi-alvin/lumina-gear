#include "lumina/belt.h"

#include <algorithm>
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

void drawCrossedBelt(Framebuffer& fb, float c1x, float c1y, int r1, float c2x, float c2y, int r2,
                     int thickness, Color c) {
    const float dx = c2x - c1x, dy = c2y - c1y;
    const float d = std::sqrt(dx * dx + dy * dy);
    if (d <= static_cast<float>(r1 + r2) + 2.0f) return;

    // Internal common tangents cross in the middle, reversing rotation direction
    const float phi = std::atan2(dy, dx);
    const float off = std::acos(std::clamp(static_cast<float>(r1 + r2) / d, -1.0f, 1.0f));
    constexpr float kPi = 3.14159265358979f;

    for (int side = -1; side <= 1; side += 2) {
        const float t1 = phi + side * off;
        const float t2 = phi + kPi - side * off;
        drawThickLine(fb, static_cast<int>(std::lround(c1x + r1 * std::cos(t1))),
                          static_cast<int>(std::lround(c1y + r1 * std::sin(t1))),
                          static_cast<int>(std::lround(c2x + r2 * std::cos(t2))),
                          static_cast<int>(std::lround(c2y + r2 * std::sin(t2))),
                          thickness, c);
    }
}

}  // namespace lumina
