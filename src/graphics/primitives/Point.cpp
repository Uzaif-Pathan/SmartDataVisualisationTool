#include "graphics/primitives/Point.h"

#include <cmath>

#include "graphics/primitives/Circle.h"

namespace gfx {

void Point::draw(GraphicsContext& ctx) const {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(size)) return;
    if (size <= 1.0) ctx.setPixel(roundToInt(x), roundToInt(y), color);
    else fillCircle(ctx, x, y, size / 2.0, color);
}

}  // namespace gfx
