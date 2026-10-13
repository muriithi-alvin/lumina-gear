// ui.h - the toolbar, the top bar with its buttons, and a tiny pixel font.
#pragma once
#include <string>

#include "lumina/components.h"
#include "lumina/framebuffer.h"

namespace lumina {

// Layout (pixels) shared by drawing and mouse picking.
constexpr int kTopBarHeight = 48;
constexpr int kToolbarWidth = 90;
constexpr int kToolSlotSize = 80;

struct Button {
    int x, y, w, h;
    std::string label;
};

// Fixed buttons in the top bar.
Button playButton();
Button pauseButton();
Button resetButton();

bool pointInRect(int px, int py, int x, int y, int w, int h);
bool hitButton(const Button& b, int px, int py);

// Which toolbar slot (0..3 = Motor, Gear, Pulley, Lightbulb) is under the
// mouse? Returns -1 when none.
int toolbarSlotAt(int px, int py);

// 5x7 pixel font (letters needed for the button labels).
void drawText(Framebuffer& fb, int x, int y, const std::string& text, int scale, Color c);
int textWidth(const std::string& text, int scale);

void drawButton(Framebuffer& fb, const Button& b, bool highlighted);
void drawTopBar(Framebuffer& fb);
void drawToolbar(Framebuffer& fb);

}  // namespace lumina
