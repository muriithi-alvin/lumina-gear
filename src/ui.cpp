#include "lumina/ui.h"

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
    {'A', {"01110", "10001", "10001", "11111", "10001", "10001", "10001"}},
    {'E', {"11111", "10000", "10000", "11110", "10000", "10000", "11111"}},
    {'L', {"10000", "10000", "10000", "10000", "10000", "10000", "11111"}},
    {'P', {"11110", "10001", "10001", "11110", "10000", "10000", "10000"}},
    {'R', {"11110", "10001", "10001", "11110", "10100", "10010", "10001"}},
    {'S', {"01111", "10000", "10000", "01110", "00001", "00001", "11110"}},
    {'T', {"11111", "00100", "00100", "00100", "00100", "00100", "00100"}},
    {'U', {"10001", "10001", "10001", "10001", "10001", "10001", "01110"}},
    {'Y', {"10001", "10001", "01010", "00100", "00100", "00100", "00100"}},
};

const Glyph* findGlyph(char ch) {
    for (const Glyph& g : kGlyphs) if (g.ch == ch) return &g;
    return nullptr;
}

Button makeButton(int index, const char* label) {
    return Button{kToolbarWidth + 20 + index * 130, 7, 120, 34, label};
}

}  // namespace

Button playButton() { return makeButton(0, "PLAY"); }
Button pauseButton() { return makeButton(1, "PAUSE"); }
Button resetButton() { return makeButton(2, "RESET"); }

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
    return static_cast<int>(text.size()) * 6 * scale - scale;
}

void drawText(Framebuffer& fb, int x, int y, const std::string& text, int scale, Color c) {
    for (char ch : text) {
        if (const Glyph* g = findGlyph(ch)) {
            for (int row = 0; row < 7; ++row)
                for (int col = 0; col < 5; ++col)
                    if (g->rows[row][col] == '1')
                        fillRect(fb, x + col * scale, y + row * scale, scale, scale, c);
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

void drawTopBar(Framebuffer& fb) {
    fillRect(fb, 0, 0, fb.width(), kTopBarHeight, kBarColor);
    drawLine(fb, 0, kTopBarHeight - 1, fb.width() - 1, kTopBarHeight - 1, palette::outline);
    drawButton(fb, playButton(), false);
    drawButton(fb, pauseButton(), false);
    drawButton(fb, resetButton(), false);
}

void drawToolbar(Framebuffer& fb) {
    fillRect(fb, 0, kTopBarHeight, kToolbarWidth, fb.height() - kTopBarHeight, kToolbarColor);
    drawLine(fb, kToolbarWidth - 1, kTopBarHeight, kToolbarWidth - 1, fb.height() - 1, palette::outline);

    for (int i = 0; i < 4; ++i) {
        int x, y;
        toolSlotRect(i, x, y);
        fillRect(fb, x, y, kToolSlotSize, kToolSlotSize, kSlotColor);
        drawRect(fb, x, y, kToolSlotSize, kToolSlotSize, palette::outline);
        const float cx = x + kToolSlotSize / 2.0f, cy = y + kToolSlotSize / 2.0f;
        switch (i) {
            case 0: drawComponent(fb, makeMotor(cx + 6, cy, 18)); break;
            case 1: drawComponent(fb, makeGear(cx, cy, 10)); break;
            case 2: drawComponent(fb, makePulley(cx, cy, 28)); break;
            case 3: drawComponent(fb, makeBulb(cx, cy - 8, 20, false)); break;
        }
    }
}

}  // namespace lumina
