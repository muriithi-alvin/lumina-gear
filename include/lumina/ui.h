// ui.h - toolbar, level selection buttons, TinkerCAD hover cards, and pixel font.
#pragma once
#include <string>

#include "lumina/components.h"
#include "lumina/framebuffer.h"

namespace lumina {

// Layout (pixels) shared by drawing and mouse picking.
constexpr int kTopBarHeight = 48;
constexpr int kToolbarWidth = 90;
constexpr int kToolSlotSize = 70;
constexpr int kSlotMargin = 12;
constexpr int kSlotCount = 7;
constexpr int kTotalButtons = 10;

struct Button {
    int x, y, w, h;
    std::string label;
};

// Fixed buttons in the top bar.
Button playButton();
Button pauseButton();
Button resetButton();
Button clearButton();
Button solveButton();
Button levelButton(int level); // 1 to 5

bool pointInRect(int px, int py, int x, int y, int w, int h);
bool hitButton(const Button& b, int px, int py);

int buttonAt(int px, int py);
int toolbarSlotAt(int px, int py);

void drawText(Framebuffer& fb, int x, int y, const std::string& text, int scale, Color c);
int textWidth(const std::string& text, int scale);

void drawButton(Framebuffer& fb, const Button& b, bool highlighted, bool active = false);
void drawTopBar(Framebuffer& fb, int highlightedButton = -1, int currentLevel = 1, bool levelCompleted = false);
void drawToolbar(Framebuffer& fb, int highlightedSlot = -1, int currentLevel = 1);

void drawComponentInfoCard(Framebuffer& fb, int slotIndex, int currentLevel, int mouseX, int mouseY);

}  // namespace lumina
