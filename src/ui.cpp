#include "lumina/ui.h"

#include <cctype>
#include "lumina/circle.h"
#include "lumina/line.h"
#include "lumina/polygon.h"

namespace lumina {

namespace {

const Color kBarColor{30, 40, 46};
const Color kToolbarColor{36, 48, 54};
const Color kSlotColor{52, 68, 76};
const Color kButtonColor{70, 92, 104};
const Color kButtonText{232, 238, 240};
const Color kAccent{255, 214, 90};

struct Glyph { char ch; const char* rows[7]; };

const Glyph kGlyphs[] = {
    // Uppercase alphabet
    {'A', {"01110", "10001", "10001", "11111", "10001", "10001", "10001"}},
    {'B', {"11110", "10001", "10001", "11110", "10001", "10001", "11110"}},
    {'C', {"01111", "10000", "10000", "10000", "10000", "10000", "01111"}},
    {'D', {"11110", "10001", "10001", "10001", "10001", "10001", "11110"}},
    {'E', {"11111", "10000", "10000", "11110", "10000", "10000", "11111"}},
    {'F', {"11111", "10000", "10000", "11110", "10000", "10000", "10000"}},
    {'G', {"01111", "10000", "10000", "10111", "10001", "10001", "01110"}},
    {'H', {"10001", "10001", "10001", "11111", "10001", "10001", "10001"}},
    {'I', {"11111", "00100", "00100", "00100", "00100", "00100", "11111"}},
    {'J', {"00111", "00010", "00010", "00010", "00010", "10010", "01100"}},
    {'K', {"10001", "10010", "10100", "11000", "10100", "10010", "10001"}},
    {'L', {"10000", "10000", "10000", "10000", "10000", "10000", "11111"}},
    {'M', {"10001", "11011", "10101", "10001", "10001", "10001", "10001"}},
    {'N', {"10001", "11001", "10101", "10011", "10001", "10001", "10001"}},
    {'O', {"01110", "10001", "10001", "10001", "10001", "10001", "01110"}},
    {'P', {"11110", "10001", "10001", "11110", "10000", "10000", "10000"}},
    {'Q', {"01110", "10001", "10001", "10001", "10101", "10011", "01111"}},
    {'R', {"11110", "10001", "10001", "11110", "10100", "10010", "10001"}},
    {'S', {"01111", "10000", "10000", "01110", "00001", "00001", "11110"}},
    {'T', {"11111", "00100", "00100", "00100", "00100", "00100", "00100"}},
    {'U', {"10001", "10001", "10001", "10001", "10001", "10001", "01110"}},
    {'V', {"10001", "10001", "10001", "10001", "10001", "01010", "00100"}},
    {'W', {"10001", "10001", "10001", "10101", "10101", "11011", "10001"}},
    {'X', {"10001", "10001", "01010", "00100", "01010", "10001", "10001"}},
    {'Y', {"10001", "10001", "01010", "00100", "00100", "00100", "00100"}},
    {'Z', {"11111", "00001", "00010", "00100", "01000", "10000", "11111"}},
    // Digits
    {'0', {"01110", "10011", "10101", "10001", "10101", "11001", "01110"}},
    {'1', {"00100", "01100", "00100", "00100", "00100", "00100", "01110"}},
    {'2', {"01110", "10001", "00001", "00010", "00100", "01000", "11111"}},
    {'3', {"11110", "00001", "00001", "01110", "00001", "00001", "11110"}},
    {'4', {"00010", "00110", "01010", "10010", "11111", "00010", "00010"}},
    {'5', {"11111", "10000", "10000", "11110", "00001", "00001", "11110"}},
    {'6', {"01110", "10000", "10000", "11110", "10001", "10001", "01110"}},
    {'7', {"11111", "00001", "00010", "00100", "01000", "01000", "01000"}},
    {'8', {"01110", "10001", "10001", "01110", "10001", "10001", "01110"}},
    {'9', {"01110", "10001", "10001", "01111", "00001", "00001", "01110"}},
    // Symbols
    {':', {"00000", "00100", "00100", "00000", "00100", "00100", "00000"}},
    {'-', {"00000", "00000", "00000", "11111", "00000", "00000", "00000"}},
    {'/', {"00001", "00010", "00010", "00100", "01000", "01000", "10000"}},
    {'[', {"01110", "01000", "01000", "01000", "01000", "01000", "01110"}},
    {']', {"01110", "00010", "00010", "00010", "00010", "00010", "01110"}},
    {'%', {"11001", "11010", "00100", "01000", "01011", "10011", "00000"}},
    {'!', {"00100", "00100", "00100", "00100", "00100", "00000", "00100"}},
    {'.', {"00000", "00000", "00000", "00000", "00000", "00100", "00100"}},
    {' ', {"00000", "00000", "00000", "00000", "00000", "00000", "00000"}}
};

const Glyph* findGlyph(char ch) {
    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    for (const Glyph& g : kGlyphs) {
        if (g.ch == ch) return &g;
    }
    return nullptr;
}

Button makeButton(int index, const char* label) {
    return Button{kToolbarWidth + 18 + index * 128, 7, 118, 34, label};
}

}  // namespace

Button playButton() { return makeButton(0, "PLAY"); }
Button pauseButton() { return makeButton(1, "PAUSE"); }
Button resetButton() { return makeButton(2, "RESET"); }
Button modeButton() { return makeButton(3, "MODE"); }

bool pointInRect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px < x + w && py >= y && py < y + h;
}

bool hitButton(const Button& b, int px, int py) { return pointInRect(px, py, b.x, b.y, b.w, b.h); }

static void toolSlotRect(int i, int& x, int& y) {
    x = (kToolbarWidth - kToolSlotSize) / 2;
    y = kTopBarHeight + 14 + i * (kToolSlotSize + 14);
}

int toolbarSlotAt(int px, int py) {
    for (int i = 0; i < 4; ++i) {
        int x, y;
        toolSlotRect(i, x, y);
        if (pointInRect(px, py, x, y, kToolSlotSize, kToolSlotSize)) return i;
    }
    return -1;
}

int textWidth(const std::string& text, int scale) {
    if (text.empty()) return 0;
    return static_cast<int>(text.size()) * 6 * scale - scale;
}

void drawText(Framebuffer& fb, int x, int y, const std::string& text, int scale, Color c) {
    for (char ch : text) {
        if (const Glyph* g = findGlyph(ch)) {
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (g->rows[row][col] == '1') {
                        fillRect(fb, x + col * scale, y + row * scale, scale, scale, c);
                    }
                }
            }
        }
        x += 6 * scale;
    }
}

void drawButton(Framebuffer& fb, const Button& b, bool highlighted) {
    fillRect(fb, b.x, b.y, b.w, b.h, highlighted ? kAccent : kButtonColor);
    drawRect(fb, b.x, b.y, b.w, b.h, palette::outline);
    const int scale = 2;
    drawText(fb, b.x + (b.w - textWidth(b.label, scale)) / 2, b.y + (b.h - 7 * scale) / 2,
             b.label, scale, highlighted ? palette::outline : kButtonText);
}

void drawTopBar(Framebuffer& fb, int highlightedButton, const char* modeLabel) {
    fillRect(fb, 0, 0, fb.width(), kTopBarHeight, kBarColor);
    drawLine(fb, 0, kTopBarHeight - 1, fb.width() - 1, kTopBarHeight - 1, palette::outline);
    drawButton(fb, playButton(), highlightedButton == 0);
    drawButton(fb, pauseButton(), highlightedButton == 1);
    drawButton(fb, resetButton(), highlightedButton == 2);
    
    Button mb = modeButton();
    if (modeLabel && modeLabel[0] != '\0') {
        mb.label = modeLabel;
    }
    drawButton(fb, mb, highlightedButton == 3);

    // Header badge
    drawText(fb, fb.width() - 250, 16, "VALVETRAIN TIMING", 2, palette::glow);
}

void drawToolbar(Framebuffer& fb, int highlightedSlot, bool isEngineMode) {
    fillRect(fb, 0, kTopBarHeight, kToolbarWidth, fb.height() - kTopBarHeight, kToolbarColor);
    drawLine(fb, kToolbarWidth - 1, kTopBarHeight, kToolbarWidth - 1, fb.height() - 1, palette::outline);

    for (int i = 0; i < 4; ++i) {
        int x, y;
        toolSlotRect(i, x, y);
        const bool lit = (i == highlightedSlot);
        fillRect(fb, x, y, kToolSlotSize, kToolSlotSize, lit ? kButtonColor : kSlotColor);
        drawRect(fb, x, y, kToolSlotSize, kToolSlotSize, lit ? kAccent : palette::outline);
        if (lit) drawRect(fb, x + 1, y + 1, kToolSlotSize - 2, kToolSlotSize - 2, kAccent);
        const float cx = x + kToolSlotSize / 2.0f, cy = y + kToolSlotSize / 2.0f;

        if (isEngineMode) {
            switch (i) {
                case 0: drawComponent(fb, makeGear(cx, cy, 12)); break;     // Crank gear
                case 1: drawComponent(fb, makeGear(cx, cy, 24)); break;     // Cam gear
                case 2: drawComponent(fb, makePulley(cx, cy, 24)); break;   // Tensioner
                case 3: drawComponent(fb, makeCamLobe(cx, cy, 18, 0.0f)); break; // Cam lobe
            }
        } else {
            switch (i) {
                case 0: drawComponent(fb, makeMotor(cx + 6, cy, 18)); break;
                case 1: drawComponent(fb, makeGear(cx, cy, 10)); break;
                case 2: drawComponent(fb, makePulley(cx, cy, 28)); break;
                case 3: drawComponent(fb, makeBulb(cx, cy - 8, 20, false)); break;
            }
        }
    }
}

}  // namespace lumina
