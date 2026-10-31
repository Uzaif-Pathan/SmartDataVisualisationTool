#pragma once
#include <cmath>

#include "graphics/Canvas.h"
#include "graphics/Color.h"

namespace gfx {

// Safe conversions from double to int.  Values are limited to +/- 1,000,000
// (NaN becomes 0) so absurd inputs can never overflow or hang the program.
inline double clampCoord(double v) {
    if (std::isnan(v)) return 0.0;
    if (v > 1e6) return 1e6;
    if (v < -1e6) return -1e6;
    return v;
}
inline int roundToInt(double v) { return static_cast<int>(std::lround(clampCoord(v))); }
inline int floorToInt(double v) { return static_cast<int>(std::floor(clampCoord(v))); }
inline int ceilToInt(double v) { return static_cast<int>(std::ceil(clampCoord(v))); }

// The low-level "pen".  Shapes never touch the Canvas directly; they draw
// through this class, which enforces the clip rectangle.
// Rectangles here are half-open: [x0, x1) x [y0, y1).
class GraphicsContext {
public:
    explicit GraphicsContext(Canvas& canvas);

    Canvas& canvas() { return canvas_; }
    int width() const { return canvas_.width(); }
    int height() const { return canvas_.height(); }

    // Restrict drawing to a rectangle (clamped to the canvas).
    // Later used to keep a chart's contents inside its plot area.
    void setClip(int x0, int y0, int x1, int y1);
    void clearClip();
    int clipX0() const { return clipX0_; }
    int clipY0() const { return clipY0_; }
    int clipX1() const { return clipX1_; }
    int clipY1() const { return clipY1_; }

    void setPixel(int x, int y, const Color& color);
    // Horizontal run of pixels: x from xStart up to (not including) xEnd.
    void hspan(int xStart, int xEnd, int y, const Color& color);
    void fillRect(int x0, int y0, int x1, int y1, const Color& color);

private:
    Canvas& canvas_;
    int clipX0_ = 0;
    int clipY0_ = 0;
    int clipX1_ = 0;
    int clipY1_ = 0;
};

}  // namespace gfx
