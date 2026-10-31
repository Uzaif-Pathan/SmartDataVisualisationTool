#pragma once
#include "graphics/Color.h"
#include "graphics/GraphicsContext.h"

namespace gfx {

// A dot.  size <= 1 draws a single pixel; larger sizes draw a filled circle
// with that diameter (used for scatter plot markers).
struct Point {
    double x = 0.0;
    double y = 0.0;
    Color color;
    double size = 1.0;

    void draw(GraphicsContext& ctx) const;
};

}  // namespace gfx
