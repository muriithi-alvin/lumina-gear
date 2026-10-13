// belt.h - a belt wrapped around two circles (pulleys, or a pulley and a wheel).
#pragma once
#include "lumina/framebuffer.h"

namespace lumina {

// Draws the two outer tangent lines between circle 1 (c1x, c1y, r1) and
// circle 2 (c2x, c2y, r2). Draw the belt first, then the pulleys over it,
// so the belt looks wrapped around them.
void drawBelt(Framebuffer& fb, float c1x, float c1y, int r1, float c2x, float c2y, int r2,
              int thickness, Color c);

}  // namespace lumina
