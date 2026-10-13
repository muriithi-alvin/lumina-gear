#include "lumina/polygon.h"

#include <algorithm>
#include <cmath>

#include "lumina/line.h"

namespace lumina {

void fillPolygon(Framebuffer& fb, const std::vector<Point>& pts, Color c) {
    const size_t n = pts.size();
    if (n < 3) return;

    float minY = pts[0].y, maxY = pts[0].y;
    for (const Point& p : pts) { minY = std::min(minY, p.y); maxY = std::max(maxY, p.y); }

    const int yStart = std::max(0, static_cast<int>(std::ceil(minY)));
    const int yEnd = std::min(fb.height() - 1, static_cast<int>(std::floor(maxY)));

    std::vector<float> xs;
    for (int y = yStart; y <= yEnd; ++y) {
        xs.clear();
        // Sample at the pixel centre so a vertex never counts twice.
        const float sy = y + 0.5f;
        for (size_t i = 0; i < n; ++i) {
            const Point& a = pts[i];
            const Point& b = pts[(i + 1) % n];
            if (a.y == b.y) continue;  // horizontal edge: no crossing
            const float lo = std::min(a.y, b.y), hi = std::max(a.y, b.y);
            if (sy < lo || sy >= hi) continue;
            xs.push_back(a.x + (sy - a.y) * (b.x - a.x) / (b.y - a.y));
        }
        std::sort(xs.begin(), xs.end());
        // Even-odd rule: fill between pairs of crossings.
        for (size_t i = 0; i + 1 < xs.size(); i += 2) {
            const int x0 = static_cast<int>(std::ceil(xs[i] - 0.5f));
            const int x1 = static_cast<int>(std::ceil(xs[i + 1] - 0.5f)) - 1;
            for (int x = x0; x <= x1; ++x) fb.setPixel(x, y, c);
        }
    }
}

void drawPolygon(Framebuffer& fb, const std::vector<Point>& pts, Color c) {
    const size_t n = pts.size();
    for (size_t i = 0; i < n; ++i) {
        const Point& a = pts[i];
        const Point& b = pts[(i + 1) % n];
        drawLine(fb, static_cast<int>(std::lround(a.x)), static_cast<int>(std::lround(a.y)),
                 static_cast<int>(std::lround(b.x)), static_cast<int>(std::lround(b.y)), c);
    }
}

void fillRect(Framebuffer& fb, int x, int y, int w, int h, Color c) {
    fillPolygon(fb, {{float(x), float(y)}, {float(x + w), float(y)},
                     {float(x + w), float(y + h)}, {float(x), float(y + h)}}, c);
}

void drawRect(Framebuffer& fb, int x, int y, int w, int h, Color c) {
    drawLine(fb, x, y, x + w - 1, y, c);
    drawLine(fb, x + w - 1, y, x + w - 1, y + h - 1, c);
    drawLine(fb, x + w - 1, y + h - 1, x, y + h - 1, c);
    drawLine(fb, x, y + h - 1, x, y, c);
}

}  // namespace lumina
