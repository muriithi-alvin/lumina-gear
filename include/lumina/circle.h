// circle.h - midpoint circle algorithm (integer maths only).
#pragma once
#include "lumina/framebuffer.h"

namespace lumina {

// Outline of a circle.
void drawCircle(Framebuffer& fb, int cx, int cy, int r, Color c);

// Solid disc, filled with horizontal spans found by the same algorithm.
void fillCircle(Framebuffer& fb, int cx, int cy, int r, Color c);

// Solid disc blended over what is already there (alpha in [0, 1]).
// Used for the soft glow around the lightbulb.
void fillCircleBlend(Framebuffer& fb, int cx, int cy, int r, Color c, float alpha);

}  // namespace lumina
