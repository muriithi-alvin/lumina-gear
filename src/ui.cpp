#include "lumina/ui.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include "lumina/circle.h"
#include "lumina/line.h"
#include "lumina/polygon.h"

namespace lumina {

namespace {

const Color kBarColor{28, 36, 42};
const Color kToolbarColor{34, 44, 50};
const Color kSlotColor{48, 62, 70};
const Color kButtonColor{64, 84, 96};
const Color kButtonText{232, 238, 240};
const Color kAccent{255, 214, 90};

struct Glyph { char ch; const char* rows[7]; };

const Glyph kGlyphs[] = {
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
    {':', {"00000", "00100", "00100", "00000", "00100", "00100", "00000"}},
    {'-', {"00000", "00000", "00000", "11111", "00000", "00000", "00000"}},
    {'/', {"00001", "00010", "00010", "00100", "01000", "01000", "10000"}},
    {'[', {"01110", "01000", "01000", "01000", "01000", "01000", "01110"}},
    {']', {"01110", "00010", "00010", "00010", "00010", "00010", "01110"}},
    {'%', {"11001", "11010", "00100", "01000", "01011", "10011", "00000"}},
    {'!', {"00100", "00100", "00100", "00100", "00100", "00000", "00100"}},
    {'.', {"00000", "00000", "00000", "00000", "00000", "00100", "00100"}},
    {'+', {"00000", "00100", "00100", "11111", "00100", "00100", "00000"}},
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
    return Button{kToolbarWidth + 14 + index * 104, 7, 98, 34, label};
}

void toolSlotRect(int i, int& x, int& y) {
    x = (kToolbarWidth - kToolSlotSize) / 2;
    y = kTopBarHeight + 10 + i * (kToolSlotSize + kSlotMargin);
}

}  // namespace

Button playButton() { return makeButton(0, "PLAY"); }
Button pauseButton() { return makeButton(1, "PAUSE"); }
Button resetButton() { return makeButton(2, "RESET"); }
Button clearButton() { return makeButton(3, "CLEAR"); }
Button assembleButton() { return makeButton(4, "ASSEMBLE"); }
Button modeButton() { return makeButton(5, "MODE"); }

bool pointInRect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px < x + w && py >= y && py < y + h;
}

bool hitButton(const Button& b, int px, int py) { return pointInRect(px, py, b.x, b.y, b.w, b.h); }

int toolbarSlotAt(int px, int py) {
    for (int i = 0; i < kSlotCount; ++i) {
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
    drawButton(fb, clearButton(), highlightedButton == 3);
    drawButton(fb, assembleButton(), highlightedButton == 4);

    Button mb = modeButton();
    if (modeLabel && modeLabel[0] != '\0') mb.label = modeLabel;
    drawButton(fb, mb, highlightedButton == 5);

    // Header badge
    drawText(fb, fb.width() - 220, 16, "ENERGY GENERATOR", 2, palette::glow);
}

// Clean, uniformly-scaled thumbnail icon drawing for each toolbar slot
static void drawSlotThumbnail(Framebuffer& fb, int slot, int cx, int cy) {
    switch (slot) {
        case 0: { // 12T Crank Sprocket (neat small gear, r = 17)
            fillCircle(fb, cx, cy, 17, palette::metalLight);
            drawCircle(fb, cx, cy, 17, palette::outline);
            fillCircle(fb, cx, cy, 6, palette::metalDark);
            // 6 tooth notches
            for (int i = 0; i < 6; ++i) {
                const float a = i * 3.14159f / 3.0f;
                drawLine(fb, cx, cy, static_cast<int>(cx + 19 * std::cos(a)), static_cast<int>(cy + 19 * std::sin(a)), palette::metalMid);
            }
            break;
        }
        case 1: { // 24T Cam Sprocket (neat scaled gear, r = 21)
            fillCircle(fb, cx, cy, 21, palette::metalMid);
            drawCircle(fb, cx, cy, 21, palette::outline);
            fillCircle(fb, cx, cy, 8, palette::metalLight);
            drawCircle(fb, cx, cy, 8, palette::outline);
            fillCircle(fb, cx, cy, 3, palette::metalDark);
            // Fine gear teeth
            for (int i = 0; i < 12; ++i) {
                const float a = i * 3.14159f / 6.0f;
                drawLine(fb, cx, cy, static_cast<int>(cx + 23 * std::cos(a)), static_cast<int>(cy + 23 * std::sin(a)), palette::metalLight);
            }
            break;
        }
        case 2: { // Timing Belt Tensioner Pulley (r = 16)
            fillCircle(fb, cx, cy, 16, palette::metalLight);
            drawCircle(fb, cx, cy, 16, palette::outline);
            fillCircle(fb, cx, cy, 5, palette::metalDark);
            // Pivot arm bracket
            drawLine(fb, cx - 12, cy + 12, cx, cy, palette::metalMid);
            break;
        }
        case 3: { // 4-Stroke Cylinder & Valves (miniature cylinder icon)
            fillRect(fb, cx - 12, cy - 14, 24, 28, Color(35, 45, 52));
            drawRect(fb, cx - 12, cy - 14, 24, 28, palette::outline);
            // Piston inside
            fillRect(fb, cx - 10, cy - 2, 20, 8, palette::metalLight);
            // Small valve
            drawThickLine(fb, cx - 5, cy - 14, cx - 5, cy - 8, 2, palette::metalMid);
            drawThickLine(fb, cx + 5, cy - 14, cx + 5, cy - 8, 2, palette::metalMid);
            break;
        }
        case 4: { // Alternator / Generator Unit (r = 17)
            fillCircle(fb, cx, cy, 17, palette::metalMid);
            drawCircle(fb, cx, cy, 17, palette::outline);
            fillCircle(fb, cx, cy, 12, palette::copper);
            drawCircle(fb, cx, cy, 12, palette::outline);
            fillCircle(fb, cx, cy, 6, palette::metalLight);
            // Terminals
            fillCircle(fb, cx - 7, cy - 17, 2, Color(220, 50, 40));
            fillCircle(fb, cx + 7, cy - 17, 2, Color(30, 35, 40));
            break;
        }
        case 5: { // Lightbulb (r = 15)
            fillCircle(fb, cx, cy - 3, 14, palette::bulbOn);
            drawCircle(fb, cx, cy - 3, 14, palette::outline);
            fillRect(fb, cx - 5, cy + 8, 10, 8, palette::metalMid);
            drawRect(fb, cx - 5, cy + 8, 10, 8, palette::outline);
            // Filament
            drawLine(fb, cx - 4, cy - 2, cx, cy - 6, Color(255, 120, 20));
            drawLine(fb, cx, cy - 6, cx + 4, cy - 2, Color(255, 120, 20));
            break;
        }
    }
}

void drawToolbar(Framebuffer& fb, int highlightedSlot, bool isEngineMode) {
    (void)isEngineMode;
    fillRect(fb, 0, kTopBarHeight, kToolbarWidth, fb.height() - kTopBarHeight, kToolbarColor);
    drawLine(fb, kToolbarWidth - 1, kTopBarHeight, kToolbarWidth - 1, fb.height() - 1, palette::outline);

    for (int i = 0; i < kSlotCount; ++i) {
        int x, y;
        toolSlotRect(i, x, y);
        const bool lit = (i == highlightedSlot);
        fillRect(fb, x, y, kToolSlotSize, kToolSlotSize, lit ? kButtonColor : kSlotColor);
        drawRect(fb, x, y, kToolSlotSize, kToolSlotSize, lit ? kAccent : palette::outline);
        if (lit) {
            drawRect(fb, x + 1, y + 1, kToolSlotSize - 2, kToolSlotSize - 2, kAccent);
        }

        const int cx = x + kToolSlotSize / 2;
        const int cy = y + kToolSlotSize / 2;
        drawSlotThumbnail(fb, i, cx, cy);
    }
}

// TinkerCAD / Wokwi floating component info card
void drawComponentInfoCard(Framebuffer& fb, int slotIndex, int mouseX, int mouseY) {
    (void)mouseX;
    if (slotIndex < 0 || slotIndex >= kSlotCount) return;

    struct CardData {
        const char* title;
        const char* category;
        const char* desc1;
        const char* desc2;
        const char* spec;
    };

    const CardData kCards[kSlotCount] = {
        {"12T CRANK SPROCKET", "ENGINE DRIVE", "Drive sprocket on engine crankshaft.", "Transmits power to timing belt and alternator.", "[SPECS: 12 TEETH | MODULE 6.0 | HIGH TORQUE]"},
        {"24T CAM SPROCKET", "VALVETRAIN DRIVE", "Driven gear running at exact 2:1 reduction.", "Rotates overhead camshaft at half engine speed.", "[SPECS: 24 TEETH | MODULE 6.0 | 2:1 REDUCTION]"},
        {"BELT TENSIONER", "TIMING SYSTEM", "Adjustable idler pulley for the timing belt.", "Eliminates belt slack to prevent engine crash.", "[SPECS: 48MM PULLEY | MANUAL TENSION TRACK]"},
        {"CYLINDER & VALVES", "PRIME MOVER", "4-stroke combustion chamber & poppet valves.", "Converts fuel combustion into rotating torque.", "[SPECS: 4-STROKE CYCLE | 185 HP | SPARK IGNITION]"},
        {"ALTERNATOR DYNAMO", "ENERGY CONVERSION", "Converts engine rotation into electricity.", "Generates 12V-14V DC power for lighting.", "[SPECS: 14.4V DC | 120W | COPPER WINDINGS]"},
        {"WORKSHOP BULB", "ELECTRICAL LOAD", "High-power incandescent illumination lamp.", "Lights up dark room when powered by alternator.", "[SPECS: 100W POWER | RADIAL ILLUMINATION]"}
    };

    const CardData& d = kCards[slotIndex];

    const int cardW = 320, cardH = 92;
    const int cardX = kToolbarWidth + 14;
    int cardY = mouseY - cardH / 2;
    cardY = std::clamp(cardY, kTopBarHeight + 10, fb.height() - cardH - 10);

    // Drop shadow
    fillRect(fb, cardX + 4, cardY + 4, cardW, cardH, Color(16, 22, 26));

    // Card background & bezel border
    fillRect(fb, cardX, cardY, cardW, cardH, Color(24, 32, 40));
    drawRect(fb, cardX, cardY, cardW, cardH, palette::blueprintCyan);
    drawRect(fb, cardX + 1, cardY + 1, cardW - 2, cardH - 2, Color(40, 55, 68));

    // Title in glowing gold
    drawText(fb, cardX + 10, cardY + 8, d.title, 1, palette::glow);

    // Category badge
    const int catW = textWidth(d.category, 1) + 8;
    fillRect(fb, cardX + cardW - catW - 10, cardY + 6, catW, 12, Color(40, 60, 75));
    drawRect(fb, cardX + cardW - catW - 10, cardY + 6, catW, 12, palette::blueprintCyan);
    drawText(fb, cardX + cardW - catW - 6, cardY + 9, d.category, 1, Color(140, 220, 255));

    // Divider line
    drawLine(fb, cardX + 10, cardY + 22, cardX + cardW - 10, cardY + 22, Color(50, 70, 85));

    // Descriptions
    drawText(fb, cardX + 10, cardY + 28, d.desc1, 1, Color(220, 230, 235));
    drawText(fb, cardX + 10, cardY + 42, d.desc2, 1, Color(180, 195, 205));

    // Specs footer
    drawText(fb, cardX + 10, cardY + 64, d.spec, 1, Color(100, 240, 140));
}

void drawCanvasInspectorCard(Framebuffer& fb, const Component& c, int mouseX, int mouseY, float speed, float power) {
    (void)c;
    const int cardW = 200, cardH = 65;
    int cx = mouseX + 18;
    int cy = mouseY - cardH / 2;
    if (cx + cardW > fb.width() - 10) cx = mouseX - cardW - 18;
    cy = std::clamp(cy, kTopBarHeight + 10, fb.height() - cardH - 10);

    fillRect(fb, cx + 3, cy + 3, cardW, cardH, Color(16, 22, 26));
    fillRect(fb, cx, cy, cardW, cardH, Color(22, 30, 38));
    drawRect(fb, cx, cy, cardW, cardH, palette::glow);

    drawText(fb, cx + 8, cy + 8, "COMPONENT INSPECTOR", 1, palette::glow);
    drawLine(fb, cx + 8, cy + 19, cx + cardW - 8, cy + 19, Color(50, 70, 80));

    char buf[32];
    std::snprintf(buf, sizeof(buf), "SPEED: %.1f RAD/S", speed);
    drawText(fb, cx + 8, cy + 26, buf, 1, Color(210, 225, 235));

    std::snprintf(buf, sizeof(buf), "POWER FLOW: %d%%", static_cast<int>(power * 100.0f));
    drawText(fb, cx + 8, cy + 42, buf, 1, Color(100, 240, 140));
}

}  // namespace lumina
