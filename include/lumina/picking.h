// picking.h - decide which component the mouse is over.
#pragma once

namespace lumina {

// Point-in-Circle test: is (px, py) inside the circle centred at (cx, cy)?
// Uses squared distances, so no square root is needed.
inline bool pointInCircle(int px, int py, int cx, int cy, int radius) {
    const long dx = px - cx, dy = py - cy;
    return dx * dx + dy * dy <= static_cast<long>(radius) * radius;
}

}  // namespace lumina
