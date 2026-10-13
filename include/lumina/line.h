// line.h - Bresenham's line algorithm (integer maths only).
#pragma once
#include "lumina/framebuffer.h"

namespace lumina {

// Draw a 1-pixel line from (x0, y0) to (x1, y1), for any slope and direction.
void drawLine(Framebuffer& fb, int x0, int y0, int x1, int y1, Color c);

// Thicker line: draws several parallel lines. thickness 1 = normal line.
void drawThickLine(Framebuffer& fb, int x0, int y0, int x1, int y1, int thickness, Color c);

}  // namespace lumina
