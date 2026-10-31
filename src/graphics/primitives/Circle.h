#pragma once
#include <optional>

#include "graphics/Color.h"
#include "graphics/GraphicsContext.h"

namespace gfx {

// A circle.  Set `fill` for a solid disc, `stroke` for an outline, or both.
// The outline is centred on the circle's edge (half inside, half outside).
// Invalid input (NaN, infinite, negative radius) draws nothing.
struct Circle {
    double cx = 0.0;
    double cy = 0.0;
    double radius = 1.0;
    std::optional<Color> fill;
    std::optional<Color> stroke;
    int strokeWidth = 1;

    void draw(GraphicsContext& ctx) const;
};

// Solid disc.  A pixel is drawn if its centre is within `radius` of (cx, cy).
void fillCircle(GraphicsContext& ctx, double cx, double cy, double radius, const Color& color);

// Ring of the given width centred on `radius`.
void strokeCircle(GraphicsContext& ctx, double cx, double cy, double radius,
                  double width, const Color& color);

}  // namespace gfx
