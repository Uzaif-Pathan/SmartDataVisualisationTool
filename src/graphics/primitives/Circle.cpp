#include "graphics/primitives/Circle.h"

#include <algorithm>
#include <cmath>

namespace gfx {
namespace {

constexpr double kMaxRadius = 1e6;

// Draws every pixel whose distance from (cx,cy) is between inner and outer.
// inner = 0 gives a solid disc.  Works row by row ("scanlines"): for each
// row, Pythagoras gives the left and right edge of the circle.
void fillAnnulus(GraphicsContext& ctx, double cx, double cy,
                 double outer, double inner, const Color& color) {
    outer = std::min(outer, kMaxRadius);
    inner = std::min(inner, kMaxRadius);

    if (outer < 0.5 && inner <= 0.0) {          // too small to have a body
        ctx.setPixel(roundToInt(cx), roundToInt(cy), color);
        return;
    }

    // Only visit rows that are inside the clip area.
    const int yStart = std::max(ceilToInt(cy - outer), ctx.clipY0());
    const int yEnd = std::min(floorToInt(cy + outer), ctx.clipY1() - 1);

    for (int y = yStart; y <= yEnd; ++y) {
        const double dy = y - cy;
        const double o2 = outer * outer - dy * dy;
        if (o2 < 0.0) continue;
        const double ohw = std::sqrt(o2);                // outer half-width
        const int ox0 = ceilToInt(cx - ohw);
        const int ox1 = floorToInt(cx + ohw);

        if (std::fabs(dy) < inner) {                     // this row crosses the hole
            const double ihw = std::sqrt(inner * inner - dy * dy);
            const int ix0 = ceilToInt(cx - ihw);
            const int ix1 = floorToInt(cx + ihw);
            ctx.hspan(ox0, ix0, y, color);               // left part of the ring
            ctx.hspan(ix1 + 1, ox1 + 1, y, color);       // right part of the ring
        } else {
            ctx.hspan(ox0, ox1 + 1, y, color);
        }
    }
}

}  // namespace

void fillCircle(GraphicsContext& ctx, double cx, double cy, double radius, const Color& color) {
    if (!std::isfinite(cx) || !std::isfinite(cy) || !std::isfinite(radius) || radius < 0.0)
        return;
    fillAnnulus(ctx, cx, cy, radius, 0.0, color);
}

void strokeCircle(GraphicsContext& ctx, double cx, double cy, double radius,
                  double width, const Color& color) {
    if (!std::isfinite(cx) || !std::isfinite(cy) || !std::isfinite(radius) ||
        !std::isfinite(width) || radius < 0.0 || width <= 0.0)
        return;
    fillAnnulus(ctx, cx, cy, radius + width / 2.0, std::max(0.0, radius - width / 2.0), color);
}

void Circle::draw(GraphicsContext& ctx) const {
    if (fill) fillCircle(ctx, cx, cy, radius, *fill);
    if (stroke && strokeWidth > 0) strokeCircle(ctx, cx, cy, radius, strokeWidth, *stroke);
}

}  // namespace gfx
