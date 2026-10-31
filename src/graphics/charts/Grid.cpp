#include "graphics/charts/Grid.h"

#include <cmath>

#include "graphics/primitives/Line.h"

namespace gfx {
namespace {

bool insideX(const PixelRect& a, double px) {
    return std::isfinite(px) && px >= a.left - 0.5 && px <= a.right() + 0.5;
}
bool insideY(const PixelRect& a, double py) {
    return std::isfinite(py) && py >= a.top - 0.5 && py <= a.bottom() + 0.5;
}

}  // namespace

void drawHorizontalGrid(Renderer& r, const Viewport& vp, const std::vector<double>& yTicks,
                        const Color& color) {
    const PixelRect& a = vp.area();
    if (!(a.width > 0.0) || !(a.height > 0.0)) return;
    for (double t : yTicks) {
        const double py = vp.mapY(t);
        if (!insideY(a, py)) continue;
        r.draw(Line{a.left, py, a.right(), py, color, 1});
    }
}

void drawVerticalGrid(Renderer& r, const Viewport& vp, const std::vector<double>& xTicks,
                      const Color& color) {
    const PixelRect& a = vp.area();
    if (!(a.width > 0.0) || !(a.height > 0.0)) return;
    for (double t : xTicks) {
        const double px = vp.mapX(t);
        if (!insideX(a, px)) continue;
        r.draw(Line{px, a.top, px, a.bottom(), color, 1});
    }
}

void drawZeroLines(Renderer& r, const Viewport& vp, const Color& color) {
    const PixelRect& a = vp.area();
    if (!(a.width > 0.0) || !(a.height > 0.0)) return;
    if (vp.yMin() < 0.0 && vp.yMax() > 0.0) {
        const double py = vp.mapY(0.0);
        if (insideY(a, py)) r.draw(Line{a.left, py, a.right(), py, color, 1});
    }
    if (vp.xMin() < 0.0 && vp.xMax() > 0.0) {
        const double px = vp.mapX(0.0);
        if (insideX(a, px)) r.draw(Line{px, a.top, px, a.bottom(), color, 1});
    }
}

}  // namespace gfx
