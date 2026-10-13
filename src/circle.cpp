#include "lumina/circle.h"

#include <vector>

namespace lumina {

namespace {

// Walk one octant with the midpoint decision variable and call
// plot(x, y) for each (x, y) offset; the caller mirrors it to all 8 octants.
template <typename Plot>
void midpointOctant(int r, Plot plot) {
    int x = 0, y = r;
    int d = 1 - r;  // decision variable at the midpoint
    while (x <= y) {
        plot(x, y);
        if (d < 0) {
            d += 2 * x + 3;          // midpoint inside: keep y
        } else {
            d += 2 * (x - y) + 5;    // midpoint outside: step y down
            --y;
        }
        ++x;
    }
}

// For every row offset 0..r, the widest x offset the midpoint walk reaches.
std::vector<int> rowHalfWidths(int r) {
    std::vector<int> half(static_cast<size_t>(r) + 1, 0);
    midpointOctant(r, [&](int x, int y) {
        if (x > half[y]) half[y] = x;   // row at offset y: half-width x
        if (y > half[x]) half[x] = y;   // row at offset x: half-width y
    });
    return half;
}

void hspan(Framebuffer& fb, int x0, int x1, int y, Color c) {
    for (int x = x0; x <= x1; ++x) fb.setPixel(x, y, c);
}

void hspanBlend(Framebuffer& fb, int x0, int x1, int y, Color c, float a) {
    for (int x = x0; x <= x1; ++x) fb.blendPixel(x, y, c, a);
}

}  // namespace

void drawCircle(Framebuffer& fb, int cx, int cy, int r, Color c) {
    if (r < 0) return;
    midpointOctant(r, [&](int x, int y) {
        fb.setPixel(cx + x, cy + y, c); fb.setPixel(cx - x, cy + y, c);
        fb.setPixel(cx + x, cy - y, c); fb.setPixel(cx - x, cy - y, c);
        fb.setPixel(cx + y, cy + x, c); fb.setPixel(cx - y, cy + x, c);
        fb.setPixel(cx + y, cy - x, c); fb.setPixel(cx - y, cy - x, c);
    });
}

void fillCircle(Framebuffer& fb, int cx, int cy, int r, Color c) {
    if (r < 0) return;
    const std::vector<int> half = rowHalfWidths(r);
    for (int row = 0; row <= r; ++row) {
        hspan(fb, cx - half[row], cx + half[row], cy + row, c);
        if (row != 0) hspan(fb, cx - half[row], cx + half[row], cy - row, c);
    }
}

void fillCircleBlend(Framebuffer& fb, int cx, int cy, int r, Color c, float alpha) {
    if (r < 0) return;
    // Every row is blended exactly once (blending twice would make the colour stronger).
    const std::vector<int> half = rowHalfWidths(r);
    for (int row = 0; row <= r; ++row) {
        hspanBlend(fb, cx - half[row], cx + half[row], cy + row, c, alpha);
        if (row != 0) hspanBlend(fb, cx - half[row], cx + half[row], cy - row, c, alpha);
    }
}

}  // namespace lumina
