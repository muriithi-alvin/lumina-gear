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

const Color kBarColor{24, 32, 38};
const Color kToolbarColor{30, 40, 46};
const Color kSlotColor{44, 58, 66};
const Color kButtonColor{58, 76, 88};
const Color kButtonActive{40, 140, 180};
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
    {'>', {"10000", "01000", "00100", "00010", "00100", "01000", "10000"}},
    {' ', {"00000", "00000", "00000", "00000", "00000", "00000", "00000"}}
};

const Glyph* findGlyph(char ch) {
    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    for (const Glyph& g : kGlyphs) {
        if (g.ch == ch) return &g;
    }
    return nullptr;
}

void toolSlotRect(int i, int& x, int& y) {
    x = (kToolbarWidth - kToolSlotSize) / 2;
    y = kTopBarHeight + 14 + i * (kToolSlotSize + kSlotMargin);
}

}  // namespace

Button playButton() { return Button{105, 8, 70, 32, "PLAY"}; }
Button pauseButton() { return Button{183, 8, 70, 32, "PAUSE"}; }
Button resetButton() { return Button{261, 8, 70, 32, "RESET"}; }
Button clearButton() { return Button{339, 8, 70, 32, "CLEAR"}; }
Button solveButton() { return Button{417, 8, 70, 32, "SOLVE"}; }

Button levelButton(int level) {
    const int startX = 505;
    const int w = 56;
    const int gap = 6;
    const int x = startX + (level - 1) * (w + gap);
    char buf[8];
    std::snprintf(buf, sizeof(buf), "LVL %d", level);
    return Button{x, 8, w, 32, buf};
}

bool pointInRect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px < x + w && py >= y && py < y + h;
}

bool hitButton(const Button& b, int px, int py) { return pointInRect(px, py, b.x, b.y, b.w, b.h); }

int buttonAt(int px, int py) {
    if (hitButton(playButton(), px, py)) return 0;
    if (hitButton(pauseButton(), px, py)) return 1;
    if (hitButton(resetButton(), px, py)) return 2;
    if (hitButton(clearButton(), px, py)) return 3;
    if (hitButton(solveButton(), px, py)) return 4;
    for (int lvl = 1; lvl <= 5; ++lvl) {
        if (hitButton(levelButton(lvl), px, py)) return 4 + lvl;
    }
    return -1;
}

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

void drawButton(Framebuffer& fb, const Button& b, bool highlighted, bool active) {
    Color bg = active ? kButtonActive : (highlighted ? kAccent : kButtonColor);
    Color fg = (highlighted && !active) ? palette::outline : kButtonText;
    fillRect(fb, b.x, b.y, b.w, b.h, bg);
    drawRect(fb, b.x, b.y, b.w, b.h, palette::outline);
    if (active) {
        drawRect(fb, b.x + 1, b.y + 1, b.w - 2, b.h - 2, Color(255, 230, 100));
    }
    const int scale = 1;
    drawText(fb, b.x + (b.w - textWidth(b.label, scale)) / 2, b.y + (b.h - 7 * scale) / 2,
             b.label, scale, fg);
}

void drawTopBar(Framebuffer& fb, int highlightedButton, int currentLevel, bool levelCompleted) {
    fillRect(fb, 0, 0, fb.width(), kTopBarHeight, kBarColor);
    drawLine(fb, 0, kTopBarHeight - 1, fb.width() - 1, kTopBarHeight - 1, palette::outline);

    drawButton(fb, playButton(), highlightedButton == 0);
    drawButton(fb, pauseButton(), highlightedButton == 1);
    drawButton(fb, resetButton(), highlightedButton == 2);
    drawButton(fb, clearButton(), highlightedButton == 3);
    drawButton(fb, solveButton(), highlightedButton == 4);

    for (int lvl = 1; lvl <= 5; ++lvl) {
        drawButton(fb, levelButton(lvl), highlightedButton == (4 + lvl), currentLevel == lvl);
    }

    // Level status badge on right side
    if (levelCompleted) {
        drawText(fb, fb.width() - 260, 18, "LEVEL SOLVED! [SUCCESS]", 1, Color(100, 240, 140));
    } else {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "LEVEL %d IN PROGRESS", currentLevel);
        drawText(fb, fb.width() - 260, 18, buf, 1, palette::glow);
    }
}

static void drawSlotThumbnail(Framebuffer& fb, int slot, int currentLevel, int cx, int cy) {
    if (currentLevel <= 3) {
        // Levels 1-3: Classic CP1/CP2 Machine Parts + 14T Reverse Idler
        switch (slot) {
            case 0: { // Electric Motor (r = 18)
                fillCircle(fb, cx, cy, 18, palette::motorBody);
                drawCircle(fb, cx, cy, 18, palette::outline);
                fillRect(fb, cx - 22, cy - 10, 8, 20, palette::motorCap);
                drawRect(fb, cx - 22, cy - 10, 8, 20, palette::outline);
                fillCircle(fb, cx, cy, 5, palette::metalLight);
                break;
            }
            case 1: { // 12T Small Gear (r = 16)
                fillCircle(fb, cx, cy, 16, palette::metalLight);
                drawCircle(fb, cx, cy, 16, palette::outline);
                fillCircle(fb, cx, cy, 5, palette::metalDark);
                for (int i = 0; i < 6; ++i) {
                    const float a = i * 3.14159f / 3.0f;
                    drawLine(fb, cx, cy, static_cast<int>(cx + 18 * std::cos(a)), static_cast<int>(cy + 18 * std::sin(a)), palette::metalMid);
                }
                break;
            }
            case 2: { // 18T Large Gear (r = 20)
                fillCircle(fb, cx, cy, 20, palette::metalMid);
                drawCircle(fb, cx, cy, 20, palette::outline);
                fillCircle(fb, cx, cy, 7, palette::metalLight);
                fillCircle(fb, cx, cy, 3, palette::metalDark);
                for (int i = 0; i < 8; ++i) {
                    const float a = i * 3.14159f / 4.0f;
                    drawLine(fb, cx, cy, static_cast<int>(cx + 22 * std::cos(a)), static_cast<int>(cy + 22 * std::sin(a)), palette::metalLight);
                }
                break;
            }
            case 3: { // 14T Reverse Idler Gear (r = 18)
                fillCircle(fb, cx, cy, 18, Color(120, 160, 190));
                drawCircle(fb, cx, cy, 18, palette::outline);
                fillCircle(fb, cx, cy, 6, palette::metalLight);
                fillCircle(fb, cx, cy, 3, palette::metalDark);
                for (int i = 0; i < 7; ++i) {
                    const float a = i * 2.0f * 3.14159f / 7.0f;
                    drawLine(fb, cx, cy, static_cast<int>(cx + 20 * std::cos(a)), static_cast<int>(cy + 20 * std::sin(a)), Color(255, 220, 100));
                }
                break;
            }
            case 4: { // 40px Large Pulley (r = 19)
                fillCircle(fb, cx, cy, 19, palette::metalLight);
                drawCircle(fb, cx, cy, 19, palette::outline);
                fillCircle(fb, cx, cy, 13, palette::metalDark);
                fillCircle(fb, cx, cy, 5, palette::metalLight);
                break;
            }
            case 5: { // 22px Small Belt Wheel (r = 14)
                fillCircle(fb, cx, cy, 14, palette::metalMid);
                drawCircle(fb, cx, cy, 14, palette::outline);
                fillCircle(fb, cx, cy, 9, palette::metalDark);
                fillCircle(fb, cx, cy, 4, palette::metalLight);
                break;
            }
            case 6: { // Lightbulb (r = 15)
                fillCircle(fb, cx, cy - 3, 14, palette::bulbOn);
                drawCircle(fb, cx, cy - 3, 14, palette::outline);
                fillRect(fb, cx - 5, cy + 8, 10, 8, palette::metalMid);
                drawRect(fb, cx - 5, cy + 8, 10, 8, palette::outline);
                drawLine(fb, cx - 4, cy - 2, cx, cy - 6, Color(255, 120, 20));
                drawLine(fb, cx, cy - 6, cx + 4, cy - 2, Color(255, 120, 20));
                break;
            }
        }
    } else {
        // Levels 4-5: 4-Stroke Engine Valvetrain & Electricity
        switch (slot) {
            case 0: { // 12T Crank Sprocket
                fillCircle(fb, cx, cy, 16, palette::metalLight);
                drawCircle(fb, cx, cy, 16, palette::outline);
                fillCircle(fb, cx, cy, 5, palette::metalDark);
                for (int i = 0; i < 6; ++i) {
                    const float a = i * 3.14159f / 3.0f;
                    drawLine(fb, cx, cy, static_cast<int>(cx + 18 * std::cos(a)), static_cast<int>(cy + 18 * std::sin(a)), palette::metalMid);
                }
                break;
            }
            case 1: { // 24T Cam Sprocket (2:1 ratio)
                fillCircle(fb, cx, cy, 21, palette::metalMid);
                drawCircle(fb, cx, cy, 21, palette::outline);
                fillCircle(fb, cx, cy, 8, palette::metalLight);
                fillCircle(fb, cx, cy, 3, palette::metalDark);
                for (int i = 0; i < 12; ++i) {
                    const float a = i * 3.14159f / 6.0f;
                    drawLine(fb, cx, cy, static_cast<int>(cx + 23 * std::cos(a)), static_cast<int>(cy + 23 * std::sin(a)), palette::metalLight);
                }
                break;
            }
            case 2: { // 12T Test Sprocket (1:1 ratio -> triggers crash!)
                fillCircle(fb, cx, cy, 16, Color(220, 70, 70));
                drawCircle(fb, cx, cy, 16, palette::outline);
                fillCircle(fb, cx, cy, 5, palette::metalDark);
                for (int i = 0; i < 6; ++i) {
                    const float a = i * 3.14159f / 3.0f;
                    drawLine(fb, cx, cy, static_cast<int>(cx + 18 * std::cos(a)), static_cast<int>(cy + 18 * std::sin(a)), Color(255, 200, 200));
                }
                break;
            }
            case 3: { // Timing Belt Tensioner Pulley
                fillCircle(fb, cx, cy, 16, palette::metalLight);
                drawCircle(fb, cx, cy, 16, palette::outline);
                fillCircle(fb, cx, cy, 5, palette::metalDark);
                drawLine(fb, cx - 12, cy + 12, cx, cy, palette::metalMid);
                break;
            }
            case 4: { // 4-Stroke Cylinder & Valves
                fillRect(fb, cx - 12, cy - 14, 24, 28, Color(35, 45, 52));
                drawRect(fb, cx - 12, cy - 14, 24, 28, palette::outline);
                fillRect(fb, cx - 10, cy - 2, 20, 8, palette::metalLight);
                drawThickLine(fb, cx - 5, cy - 14, cx - 5, cy - 8, 2, palette::metalMid);
                drawThickLine(fb, cx + 5, cy - 14, cx + 5, cy - 8, 2, palette::metalMid);
                break;
            }
            case 5: { // Alternator / Dynamo Generator
                fillCircle(fb, cx, cy, 17, palette::metalMid);
                drawCircle(fb, cx, cy, 17, palette::outline);
                fillCircle(fb, cx, cy, 12, palette::copper);
                drawCircle(fb, cx, cy, 12, palette::outline);
                fillCircle(fb, cx, cy, 6, palette::metalLight);
                fillCircle(fb, cx - 7, cy - 17, 2, Color(220, 50, 40));
                fillCircle(fb, cx + 7, cy - 17, 2, Color(30, 35, 40));
                break;
            }
            case 6: { // Lightbulb
                fillCircle(fb, cx, cy - 3, 14, palette::bulbOn);
                drawCircle(fb, cx, cy - 3, 14, palette::outline);
                fillRect(fb, cx - 5, cy + 8, 10, 8, palette::metalMid);
                drawRect(fb, cx - 5, cy + 8, 10, 8, palette::outline);
                drawLine(fb, cx - 4, cy - 2, cx, cy - 6, Color(255, 120, 20));
                drawLine(fb, cx, cy - 6, cx + 4, cy - 2, Color(255, 120, 20));
                break;
            }
        }
    }
}

void drawToolbar(Framebuffer& fb, int highlightedSlot, int currentLevel) {
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
        drawSlotThumbnail(fb, i, currentLevel, cx, cy);
    }
}

void drawComponentInfoCard(Framebuffer& fb, int slotIndex, int currentLevel, int mouseX, int mouseY) {
    (void)mouseX;
    if (slotIndex < 0 || slotIndex >= kSlotCount) return;

    struct CardData {
        const char* title;
        const char* category;
        const char* desc1;
        const char* desc2;
        const char* spec;
    };

    CardData d;

    if (currentLevel <= 3) {
        const CardData kClassicCards[7] = {
            {"PRIME MOVER MOTOR", "POWER SOURCE", "Rotates clockwise delivering mechanical torque.", "Must be connected to gear train to drive machine.", "[SPECS: 4.0 RAD/S | 100% INITIAL TORQUE]"},
            {"12T DRIVE GEAR", "SPEED TRANSMISSION", "Small spur gear. Meshes with 18T gear.", "Opposite rotational direction on mesh.", "[SPECS: 12 TEETH | PITCH RADIUS 36PX]"},
            {"18T DRIVEN GEAR", "GEAR REDUCTION", "Large spur gear (18 teeth).", "Output speed = 12/18 = 0.67x input speed.", "[SPECS: 18 TEETH | PITCH RADIUS 54PX]"},
            {"14T IDLER GEAR", "DIRECTION REVERSER", "Intermediate gear placed between Gear 1 and 2.", "Reverses rotation to Clockwise without altering ratio!", "[SPECS: 14 TEETH | FIXES LEVEL 3 POLARITY]"},
            {"40PX BELT PULLEY", "BELT TRANSMISSION", "Mounted on same axle as 18T gear.", "Drives belt to small generator wheel.", "[SPECS: RADIUS 40PX | SPEED STEP-UP]"},
            {"22PX BELT WHEEL", "SPEED MULTIPLIER", "Driven by belt from 40px pulley.", "Multiplies speed by 40/22 = 1.82x ratio!", "[SPECS: RADIUS 22PX | POWERS GENERATOR]"},
            {"LIGHTBULB", "ELECTRICAL LOAD", "Converts generator mechanical power into light.", "Illuminates dark room when power reaches it.", "[SPECS: 100% GLOW AT TARGET SPEED]"}
        };
        d = kClassicCards[slotIndex];
    } else {
        const CardData kEngineCards[7] = {
            {"12T CRANK SPROCKET", "ENGINE DRIVE", "Drive sprocket on engine crankshaft.", "Transmits power to timing belt and alternator.", "[SPECS: 12 TEETH | MODULE 6.0 | HIGH TORQUE]"},
            {"24T CAM SPROCKET", "VALVETRAIN DRIVE", "Driven gear running at exact 2:1 reduction.", "Rotates overhead camshaft at half engine speed.", "[SPECS: 24 TEETH | NEEDED TO AVOID CRASH!]"},
            {"12T TEST SPROCKET", "CRASH SIMULATOR", "1:1 ratio gear (spins cam at full engine speed).", "Desynchronizes valves causing fatal piston crash!", "[SPECS: 12 TEETH | TRIGGERS VALVE IMPACT]"},
            {"BELT TENSIONER", "TIMING SYSTEM", "Adjustable idler pulley for the timing belt.", "Eliminates belt slack to prevent engine crash.", "[SPECS: 48MM PULLEY | MANUAL TENSION TRACK]"},
            {"CYLINDER & VALVES", "PRIME MOVER", "4-stroke combustion chamber & poppet valves.", "Converts fuel combustion into rotating torque.", "[SPECS: 4-STROKE CYCLE | 185 HP | SPARK IGNITION]"},
            {"ALTERNATOR DYNAMO", "ENERGY CONVERSION", "Converts engine rotation into electricity.", "Generates 12V-14V DC power for lighting.", "[SPECS: 14.4V DC | 120W | COPPER WINDINGS]"},
            {"WORKSHOP BULB", "ELECTRICAL LOAD", "High-power incandescent illumination lamp.", "Lights up dark room when powered by alternator.", "[SPECS: 100W POWER | RADIAL ILLUMINATION]"}
        };
        d = kEngineCards[slotIndex];
    }

    const int cardW = 340, cardH = 92;
    const int cardX = kToolbarWidth + 14;
    int cardY = mouseY - cardH / 2;
    cardY = std::clamp(cardY, kTopBarHeight + 10, fb.height() - cardH - 10);

    fillRect(fb, cardX + 4, cardY + 4, cardW, cardH, Color(14, 18, 22));
    fillRect(fb, cardX, cardY, cardW, cardH, Color(22, 30, 36));
    drawRect(fb, cardX, cardY, cardW, cardH, Color(70, 190, 220));
    drawRect(fb, cardX + 1, cardY + 1, cardW - 2, cardH - 2, Color(38, 52, 64));

    drawText(fb, cardX + 10, cardY + 8, d.title, 1, palette::glow);

    const int catW = textWidth(d.category, 1) + 8;
    fillRect(fb, cardX + cardW - catW - 10, cardY + 6, catW, 12, Color(38, 56, 70));
    drawRect(fb, cardX + cardW - catW - 10, cardY + 6, catW, 12, Color(70, 190, 220));
    drawText(fb, cardX + cardW - catW - 6, cardY + 9, d.category, 1, Color(140, 220, 255));

    drawLine(fb, cardX + 10, cardY + 22, cardX + cardW - 10, cardY + 22, Color(48, 66, 78));
    drawText(fb, cardX + 10, cardY + 28, d.desc1, 1, Color(220, 230, 235));
    drawText(fb, cardX + 10, cardY + 42, d.desc2, 1, Color(180, 195, 205));
    drawText(fb, cardX + 10, cardY + 64, d.spec, 1, Color(100, 240, 140));
}

}  // namespace lumina
