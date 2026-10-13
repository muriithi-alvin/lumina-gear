// Simple self-checking tests (no test library needed).
// Run with:  ctest --output-on-failure
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "lumina/belt.h"
#include "lumina/circle.h"
#include "lumina/components.h"
#include "lumina/effects.h"
#include "lumina/line.h"
#include "lumina/picking.h"
#include "lumina/polygon.h"
#include "lumina/ui.h"

using namespace lumina;

static int failures = 0;
#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) { std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n"; ++failures; } \
    } while (0)

static const Color W(255, 255, 255), K(0, 0, 0);

static bool isLit(const Framebuffer& fb, int x, int y) { return fb.getPixel(x, y).r == 255; }

static int countLit(const Framebuffer& fb) {
    int n = 0;
    for (int y = 0; y < fb.height(); ++y)
        for (int x = 0; x < fb.width(); ++x) n += isLit(fb, x, y);
    return n;
}

// Bresenham draws exactly max(|dx|, |dy|) + 1 pixels, each within 0.5 pixel
// (vertically or horizontally) of the ideal straight line.
static void testLineAccuracy() {
    unsigned seed = 12345;
    auto rnd = [&](int n) { seed = seed * 1103515245u + 12345u; return int((seed >> 16) % n); };
    for (int i = 0; i < 300; ++i) {
        const int x0 = rnd(90) + 5, y0 = rnd(90) + 5, x1 = rnd(90) + 5, y1 = rnd(90) + 5;
        Framebuffer fb(100, 100);
        fb.clear(K);
        drawLine(fb, x0, y0, x1, y1, W);
        const int dx = x1 - x0, dy = y1 - y0;
        CHECK(countLit(fb) == std::max(std::abs(dx), std::abs(dy)) + 1);
        for (int y = 0; y < 100; ++y)
            for (int x = 0; x < 100; ++x)
                if (isLit(fb, x, y)) {
                    // Perpendicular offset from the ideal line (cross product / length).
                    const double len = std::hypot(dx, dy);
                    const double off = len == 0 ? 0 : std::fabs(dx * double(y - y0) - dy * double(x - x0)) / len;
                    CHECK(off <= 0.71);
                }
    }
}

static void testLine() {
    // Horizontal, vertical, diagonal and a steep line in every direction.
    Framebuffer fb(50, 50);
    fb.clear(K);
    drawLine(fb, 5, 5, 20, 5, W);
    CHECK(countLit(fb) == 16);

    fb.clear(K);
    drawLine(fb, 5, 5, 5, 20, W);
    CHECK(countLit(fb) == 16);

    fb.clear(K);
    drawLine(fb, 5, 5, 20, 20, W);
    CHECK(countLit(fb) == 16);
    CHECK(isLit(fb, 12, 12));

    // Endpoints are always drawn, whichever way round the line is given.
    const int pts[][4] = {{3, 40, 45, 10}, {45, 10, 3, 40}, {10, 3, 12, 45}, {12, 45, 10, 3},
                          {40, 40, 2, 38}, {2, 2, 2, 2}};
    for (const auto& p : pts) {
        fb.clear(K);
        drawLine(fb, p[0], p[1], p[2], p[3], W);
        CHECK(isLit(fb, p[0], p[1]));
        CHECK(isLit(fb, p[2], p[3]));
    }

    // A reversed line covers exactly the same pixels in count.
    fb.clear(K); drawLine(fb, 3, 40, 45, 10, W); const int a = countLit(fb);
    fb.clear(K); drawLine(fb, 45, 10, 3, 40, W); const int b = countLit(fb);
    CHECK(a == b);

    // Drawing partly off-screen must not crash.
    drawLine(fb, -20, -20, 80, 80, W);
}

// Independent reference: for each x offset the y offset is round(sqrt(r^2 - x^2)),
// mirrored into all octants. The midpoint algorithm must produce the same pixels.
static void testCircleAgainstReference() {
    for (int r = 1; r <= 60; ++r) {
        Framebuffer fb(200, 200), fill(200, 200);
        fb.clear(K); fill.clear(K);
        drawCircle(fb, 100, 100, r, W);
        fillCircle(fill, 100, 100, r, W);
        std::vector<char> ref(200 * 200, 0);
        std::vector<int> half(static_cast<size_t>(r) + 1, 0);  // widest x for each |y|
        for (int x = 0; x <= r; ++x) {
            const int y = static_cast<int>(std::lround(std::sqrt(double(r) * r - double(x) * x)));
            for (int sx : {x, -x})
                for (int sy : {y, -y}) {
                    ref[(100 + sy) * 200 + (100 + sx)] = 1;
                    ref[(100 + sx) * 200 + (100 + sy)] = 1;
                }
            half[y] = std::max(half[y], x);
            half[x] = std::max(half[x], y);
        }
        int outlineMismatch = 0, fillMismatch = 0;
        for (int y = 0; y < 200; ++y)
            for (int x = 0; x < 200; ++x) {
                outlineMismatch += (isLit(fb, x, y) != (ref[y * 200 + x] != 0));
                const int dy = std::abs(y - 100), dx = std::abs(x - 100);
                const bool inside = dy <= r && dx <= half[dy];
                fillMismatch += (isLit(fill, x, y) != inside);
            }
        CHECK(outlineMismatch == 0);
        CHECK(fillMismatch == 0);
    }
}

static void testCircle() {
    Framebuffer fb(100, 100);
    fb.clear(K);
    drawCircle(fb, 50, 50, 20, W);
    CHECK(isLit(fb, 70, 50)); CHECK(isLit(fb, 30, 50));
    CHECK(isLit(fb, 50, 70)); CHECK(isLit(fb, 50, 30));
    CHECK(!isLit(fb, 50, 50));
    // Every lit pixel must be within half a pixel of the true radius.
    for (int y = 0; y < 100; ++y)
        for (int x = 0; x < 100; ++x)
            if (isLit(fb, x, y)) {
                const double d = std::hypot(x - 50, y - 50);
                CHECK(std::fabs(d - 20) < 1.0);
            }

    // Filled disc: the area lies between pi*(r-0.5)^2 and pi*(r+0.5)^2
    // (the outline pixels are part of the disc).
    fb.clear(K);
    fillCircle(fb, 50, 50, 30, W);
    const double area = countLit(fb);
    CHECK(area > 3.14159265 * 29.5 * 29.5 && area < 3.14159265 * 30.5 * 30.5);
    CHECK(isLit(fb, 50, 50)); CHECK(isLit(fb, 80, 50)); CHECK(!isLit(fb, 82, 50));
    for (int y = 0; y < 100; ++y)
        for (int x = 0; x < 100; ++x) {
            const double d = std::hypot(x - 50, y - 50);
            if (d < 29) CHECK(isLit(fb, x, y));
            if (d > 31) CHECK(!isLit(fb, x, y));
        }

    // Radius 0 draws a single pixel; off-screen circles do not crash.
    fb.clear(K); drawCircle(fb, 10, 10, 0, W); CHECK(countLit(fb) == 1);
    fillCircle(fb, -5, 105, 40, W);

    // Blended disc: the centre changes by exactly alpha, and only once.
    fb.clear(Color(0, 0, 0));
    fillCircleBlend(fb, 50, 50, 20, Color(200, 200, 200), 0.5f);
    CHECK(fb.getPixel(50, 50).r == 100);
    CHECK(fb.getPixel(80, 80).r == 0);
}

static void testPolygon() {
    Framebuffer fb(60, 60);
    fb.clear(K);
    fillRect(fb, 10, 10, 20, 15, W);
    CHECK(countLit(fb) == 20 * 15);
    CHECK(isLit(fb, 10, 10)); CHECK(isLit(fb, 29, 24));
    CHECK(!isLit(fb, 30, 24)); CHECK(!isLit(fb, 29, 25));

    // Triangle area is about half of the bounding box.
    fb.clear(K);
    fillPolygon(fb, {{10, 10}, {50, 10}, {10, 50}}, W);
    CHECK(std::abs(countLit(fb) - 800) < 60);

    // Concave "L" shape: the notch must stay empty (even-odd rule).
    fb.clear(K);
    fillPolygon(fb, {{10, 10}, {20, 10}, {20, 40}, {50, 40}, {50, 50}, {10, 50}}, W);
    CHECK(isLit(fb, 15, 30)); CHECK(isLit(fb, 40, 45));
    CHECK(!isLit(fb, 40, 20));

    // U shape: rows through both arms cross four edges. The gap between the
    // arms must stay empty while both arms are filled.
    fb.clear(K);
    fillPolygon(fb, {{10, 10}, {20, 10}, {20, 40}, {40, 40}, {40, 10}, {50, 10}, {50, 50}, {10, 50}}, W);
    CHECK(isLit(fb, 15, 25)); CHECK(isLit(fb, 45, 25));
    CHECK(!isLit(fb, 30, 25));
    CHECK(isLit(fb, 30, 45));

    // Vertices that sit exactly on a pixel centre (y = N.5) must be counted once,
    // otherwise the row through the vertex comes out empty.
    fb.clear(K);
    fillPolygon(fb, {{30, 10.5f}, {40, 20.5f}, {30, 30.5f}, {20, 20.5f}}, W);
    CHECK(isLit(fb, 30, 20));
    CHECK(isLit(fb, 25, 20)); CHECK(isLit(fb, 35, 20));
    CHECK(isLit(fb, 30, 15)); CHECK(isLit(fb, 30, 25));

    // Fewer than 3 points: nothing drawn.
    fb.clear(K); fillPolygon(fb, {{1, 1}, {5, 5}}, W); CHECK(countLit(fb) == 0);
}

static void testPicking() {
    CHECK(pointInCircle(100, 100, 100, 100, 10));   // centre
    CHECK(pointInCircle(110, 100, 100, 100, 10));   // exactly on the edge
    CHECK(!pointInCircle(111, 100, 100, 100, 10));  // just outside
    CHECK(pointInCircle(107, 107, 100, 100, 10));   // 9.9 away
    CHECK(!pointInCircle(108, 108, 100, 100, 10));  // 11.3 away
    CHECK(!pointInCircle(0, 0, 100, 100, 10));
}

static void testUiHits() {
    const Button play = playButton();
    CHECK(hitButton(play, play.x + 5, play.y + 5));
    CHECK(!hitButton(play, play.x - 1, play.y + 5));
    CHECK(!hitButton(play, pauseButton().x + 5, pauseButton().y + 5));
    CHECK(toolbarSlotAt(45, kTopBarHeight + 14 + 10) == 0);
    CHECK(toolbarSlotAt(45, kTopBarHeight + 14 + (80 + 14) * 3 + 10) == 3);
    CHECK(toolbarSlotAt(500, 300) == -1);
}

static void testGearsMesh() {
    // Two gears placed at the "touching" distance must not overlap in tooth
    // area: the pixels covered by both gears (drawn separately) are tiny.
    Framebuffer a(300, 200), b(300, 200);
    a.clear(K); b.clear(K);
    const float d = gearPitchRadius(12) + gearPitchRadius(18);
    drawGear(a, makeGear(100, 100, 12, 0));
    drawGear(b, makeGear(100 + d, 100, 18, meshAngle(18, 3.14159265f)));
    int overlap = 0;
    for (int y = 0; y < 200; ++y)
        for (int x = 0; x < 300; ++x)
            overlap += (a.getPixel(x, y).r != 0) && (b.getPixel(x, y).r != 0);
    CHECK(overlap < 40);
}

static void testEffects() {
    Framebuffer fb(100, 100);
    fb.clear(Color(100, 100, 100));
    applyDarkness(fb, 0, 0, 100, 100, 0.5f, {}, W);
    CHECK(fb.getPixel(50, 50).r == 50);   // no light: plain 50% black

    fb.clear(Color(100, 100, 100));
    applyDarkness(fb, 0, 0, 100, 100, 0.5f, {{50, 50, 40, 1.0f}}, Color(255, 214, 90));
    CHECK(fb.getPixel(50, 50).r > 100);   // light pierces the darkness and adds colour
    CHECK(fb.getPixel(95, 95).r == 50);   // far from the light: still dark

    fb.clear(Color(0, 0, 0));
    drawGlow(fb, 50, 50, 40, Color(255, 214, 90), 1.0f);
    CHECK(fb.getPixel(50, 50).r > fb.getPixel(50, 85).r);  // brighter in the middle
    CHECK(fb.getPixel(95, 95).r == 0);

    Framebuffer off(100, 100);
    off.clear(Color(0, 0, 0));
    drawGlow(off, 50, 50, 40, Color(255, 214, 90), 0.0f);
    CHECK(off.getPixel(50, 50).r == 0);   // intensity 0 draws nothing
}

static void testBelt() {
    Framebuffer fb(300, 200);
    fb.clear(K);
    drawBelt(fb, 60, 100, 30, 240, 100, 30, 1, W);   // equal circles: two horizontal lines
    CHECK(isLit(fb, 150, 70)); CHECK(isLit(fb, 150, 130));
    CHECK(!isLit(fb, 150, 100));
}

static void testFramebuffer() {
    Framebuffer fb(10, 10);
    fb.setPixel(-1, 0, W); fb.setPixel(10, 10, W);          // ignored, must not crash
    CHECK(fb.getPixel(-1, -1).r == 0);
    fb.setPixel(3, 4, Color(1, 2, 3));
    CHECK(fb.getPixel(3, 4).g == 2);
    fb.setPixel(0, 0, Color(0, 0, 0));
    fb.blendPixel(0, 0, W, 0.25f);
    CHECK(fb.getPixel(0, 0).r == 64);
    fb.addPixel(0, 0, W, 5.0f);
    CHECK(fb.getPixel(0, 0).r == 255);                       // clamps at 255
    CHECK(fb.saveBMP("test_output.bmp"));
    std::remove("test_output.bmp");
}

int main() {
    testFramebuffer();
    testLine();
    testLineAccuracy();
    testCircleAgainstReference();
    testCircle();
    testPolygon();
    testPicking();
    testUiHits();
    testGearsMesh();
    testEffects();
    testBelt();
    if (failures == 0) { std::cout << "All tests passed\n"; return 0; }
    std::cerr << failures << " check(s) failed\n";
    return 1;
}
