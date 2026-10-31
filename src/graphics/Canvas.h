#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "graphics/Color.h"

namespace gfx {

// A rectangular grid of RGBA pixels in memory.
// Pixel (0,0) is the TOP-LEFT corner; x grows right, y grows DOWN.
class Canvas {
public:
    // Throws std::invalid_argument if width/height is <= 0 or > 16384.
    Canvas(int width, int height, const Color& background = colors::White);

    int width() const { return width_; }
    int height() const { return height_; }

    void clear(const Color& color);
    bool inBounds(int x, int y) const;

    // Draws one pixel.  Out-of-range coordinates are silently ignored.
    // Translucent colors are blended with what is already there
    // (this assumes the canvas underneath is opaque).
    void setPixel(int x, int y, const Color& color);

    // Returns the pixel, or fully transparent (0,0,0,0) if out of range.
    Color getPixel(int x, int y) const;

    // Writes the canvas to a PNG file.  Returns false on failure
    // (e.g. the folder does not exist).  Does not create folders.
    bool savePng(const std::string& path) const;

private:
    int width_;
    int height_;
    std::vector<std::uint8_t> pixels_;   // R,G,B,A per pixel, row by row
};

}  // namespace gfx
