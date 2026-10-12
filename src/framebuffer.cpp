#include "lumina/framebuffer.h"

#include <algorithm>
#include <fstream>

namespace lumina {

namespace {
std::uint8_t clamp255(float v) {
    return static_cast<std::uint8_t>(std::min(255.0f, std::max(0.0f, v)) + 0.5f);
}
}  // namespace

Framebuffer::Framebuffer(int width, int height)
    : width_(width), height_(height), pixels_(static_cast<size_t>(width) * height) {}

void Framebuffer::clear(Color c) { std::fill(pixels_.begin(), pixels_.end(), c); }

void Framebuffer::setPixel(int x, int y, Color c) {
    if (!inBounds(x, y)) return;
    pixels_[static_cast<size_t>(y) * width_ + x] = c;
}

Color Framebuffer::getPixel(int x, int y) const {
    if (!inBounds(x, y)) return Color();
    return pixels_[static_cast<size_t>(y) * width_ + x];
}

void Framebuffer::blendPixel(int x, int y, Color src, float alpha) {
    if (!inBounds(x, y)) return;
    alpha = std::min(1.0f, std::max(0.0f, alpha));
    Color d = getPixel(x, y);
    setPixel(x, y, Color(clamp255(src.r * alpha + d.r * (1 - alpha)),
                         clamp255(src.g * alpha + d.g * (1 - alpha)),
                         clamp255(src.b * alpha + d.b * (1 - alpha))));
}

void Framebuffer::addPixel(int x, int y, Color src, float strength) {
    if (!inBounds(x, y)) return;
    Color d = getPixel(x, y);
    setPixel(x, y, Color(clamp255(d.r + src.r * strength),
                         clamp255(d.g + src.g * strength),
                         clamp255(d.b + src.b * strength)));
}

bool Framebuffer::saveBMP(const std::string& path) const {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;

    const int rowBytes = (width_ * 3 + 3) & ~3;  // rows are padded to 4 bytes
    const std::uint32_t dataSize = static_cast<std::uint32_t>(rowBytes) * height_;
    const std::uint32_t fileSize = 54 + dataSize;

    auto w16 = [&](std::uint16_t v) { f.put(v & 0xFF); f.put((v >> 8) & 0xFF); };
    auto w32 = [&](std::uint32_t v) { for (int i = 0; i < 4; ++i) f.put((v >> (8 * i)) & 0xFF); };

    f.put('B'); f.put('M');
    w32(fileSize); w32(0); w32(54);                          // file header
    w32(40); w32(width_); w32(height_); w16(1); w16(24);     // info header
    w32(0); w32(dataSize); w32(2835); w32(2835); w32(0); w32(0);

    std::vector<char> row(rowBytes, 0);
    for (int y = height_ - 1; y >= 0; --y) {                 // BMP stores bottom row first
        for (int x = 0; x < width_; ++x) {
            Color c = getPixel(x, y);
            row[x * 3 + 0] = static_cast<char>(c.b);
            row[x * 3 + 1] = static_cast<char>(c.g);
            row[x * 3 + 2] = static_cast<char>(c.r);
        }
        f.write(row.data(), rowBytes);
    }
    return static_cast<bool>(f);
}

}  // namespace lumina
