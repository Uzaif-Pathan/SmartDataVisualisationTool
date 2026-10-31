#include "graphics/Canvas.h"

#include <stdexcept>

// This is the ONE place that compiles the PNG-writing code from stb.
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

namespace gfx {

Canvas::Canvas(int width, int height, const Color& background)
    : width_(width), height_(height) {
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Canvas: width and height must be positive");
    if (width > 16384 || height > 16384)
        throw std::invalid_argument("Canvas: size too large (maximum 16384)");
    pixels_.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4);
    clear(background);
}

void Canvas::clear(const Color& c) {
    for (std::size_t i = 0; i < pixels_.size(); i += 4) {
        pixels_[i] = c.r;
        pixels_[i + 1] = c.g;
        pixels_[i + 2] = c.b;
        pixels_[i + 3] = c.a;
    }
}

bool Canvas::inBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < width_ && y < height_;
}

void Canvas::setPixel(int x, int y, const Color& c) {
    if (!inBounds(x, y) || c.a == 0) return;
    const std::size_t i =
        (static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
         static_cast<std::size_t>(x)) * 4;
    if (c.a == 255) {
        pixels_[i] = c.r;
        pixels_[i + 1] = c.g;
        pixels_[i + 2] = c.b;
        pixels_[i + 3] = 255;
        return;
    }
    // Blend: result = (source * a + existing * (255 - a)) / 255
    const int a = c.a;
    pixels_[i]     = static_cast<std::uint8_t>((c.r * a + pixels_[i]     * (255 - a)) / 255);
    pixels_[i + 1] = static_cast<std::uint8_t>((c.g * a + pixels_[i + 1] * (255 - a)) / 255);
    pixels_[i + 2] = static_cast<std::uint8_t>((c.b * a + pixels_[i + 2] * (255 - a)) / 255);
}

Color Canvas::getPixel(int x, int y) const {
    if (!inBounds(x, y)) return Color(0, 0, 0, 0);
    const std::size_t i =
        (static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
         static_cast<std::size_t>(x)) * 4;
    return Color(pixels_[i], pixels_[i + 1], pixels_[i + 2], pixels_[i + 3]);
}

bool Canvas::savePng(const std::string& path) const {
    return stbi_write_png(path.c_str(), width_, height_, 4, pixels_.data(), width_ * 4) != 0;
}

}  // namespace gfx
