// framebuffer.h - the pixel buffer every drawing routine writes into.
// Pure C++: no graphics library is used. The finished image is saved as a
// BMP file, which Windows can open directly.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace lumina {

struct Color {
    std::uint8_t r = 0, g = 0, b = 0;
    constexpr Color() = default;
    constexpr Color(std::uint8_t r_, std::uint8_t g_, std::uint8_t b_) : r(r_), g(g_), b(b_) {}
};

class Framebuffer {
public:
    Framebuffer(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

    bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < width_ && y < height_; }

    void clear(Color c);

    // Replace a pixel. Out-of-bounds writes are ignored.
    void setPixel(int x, int y, Color c);
    Color getPixel(int x, int y) const;

    // Alpha blend: result = src * alpha + dst * (1 - alpha), alpha in [0, 1].
    void blendPixel(int x, int y, Color src, float alpha);

    // Additive blend: result = dst + src * strength (clamped to 255).
    void addPixel(int x, int y, Color src, float strength);

    // Save as a 24-bit BMP file. Returns false if the file cannot be written.
    bool saveBMP(const std::string& path) const;

private:
    int width_, height_;
    std::vector<Color> pixels_;
};

}  // namespace lumina
