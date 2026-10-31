#include "graphics/primitives/Line.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "graphics/primitives/Circle.h"

namespace gfx {

void Line::draw(GraphicsContext& ctx) const {
    if (!std::isfinite(x0) || !std::isfinite(y0) || !std::isfinite(x1) || !std::isfinite(y1))
        return;

    int ix0 = roundToInt(x0);
    int iy0 = roundToInt(y0);
    const int ix1 = roundToInt(x1);
    const int iy1 = roundToInt(y1);

    const int t = std::min(thickness, 1000);
    const double stampRadius = t / 2.0;

    const int dx = std::abs(ix1 - ix0);
    const int sx = ix0 < ix1 ? 1 : -1;
    const int dy = -std::abs(iy1 - iy0);
    const int sy = iy0 < iy1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        if (t <= 1) ctx.setPixel(ix0, iy0, color);
        else fillCircle(ctx, ix0, iy0, stampRadius, color);

        if (ix0 == ix1 && iy0 == iy1) break;
        const int e2 = 2 * err;
        if (e2 >= dy) { err += dy; ix0 += sx; }
        if (e2 <= dx) { err += dx; iy0 += sy; }
    }
}

}  // namespace gfx
