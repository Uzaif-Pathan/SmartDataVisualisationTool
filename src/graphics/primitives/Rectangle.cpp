#include "graphics/primitives/Rectangle.h"

#include <algorithm>
#include <cmath>

namespace gfx {

void Rectangle::draw(GraphicsContext& ctx) const {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) ||
        !std::isfinite(height))
        return;
    if (width <= 0.0 || height <= 0.0) return;

    const int x0 = roundToInt(x);
    const int y0 = roundToInt(y);
    int x1 = roundToInt(x + width);
    int y1 = roundToInt(y + height);
    if (x1 <= x0) x1 = x0 + 1;
    if (y1 <= y0) y1 = y0 + 1;

    if (fill) ctx.fillRect(x0, y0, x1, y1, *fill);

    if (stroke && strokeWidth > 0) {
        const int sw = std::min(strokeWidth, 100000);
        if (2 * sw >= x1 - x0 || 2 * sw >= y1 - y0) {
            ctx.fillRect(x0, y0, x1, y1, *stroke);            // too thin for a hollow centre
        } else {
            ctx.fillRect(x0, y0, x1, y0 + sw, *stroke);       // top
            ctx.fillRect(x0, y1 - sw, x1, y1, *stroke);       // bottom
            ctx.fillRect(x0, y0 + sw, x0 + sw, y1 - sw, *stroke);   // left
            ctx.fillRect(x1 - sw, y0 + sw, x1, y1 - sw, *stroke);   // right
        }
    }
}

}  // namespace gfx
