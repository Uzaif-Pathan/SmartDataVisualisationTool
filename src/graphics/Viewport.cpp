#include "graphics/Viewport.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace gfx {
namespace {

constexpr double kMaxPixel = 1e7;

double cleanLength(double v) { return (std::isfinite(v) && v > 0.0) ? v : 0.0; }
double cleanCoord(double v) { return std::isfinite(v) ? v : 0.0; }
double clampPixel(double v) { return std::max(-kMaxPixel, std::min(kMaxPixel, v)); }

// Makes sure lo < hi and both are finite.  Returns true if it changed anything.
bool repairRange(double& lo, double& hi) {
    if (!std::isfinite(lo) || !std::isfinite(hi)) {
        lo = 0.0;
        hi = 1.0;
        return true;
    }
    bool changed = false;
    if (lo > hi) {
        std::swap(lo, hi);
        changed = true;
    }
    if (lo == hi) {
        const double delta = (lo == 0.0) ? 1.0 : std::fabs(lo) * 0.1;
        lo -= delta;
        hi += delta;
        changed = true;
    }
    // Last resort (e.g. subnormal numbers, or a span too big for a double).
    if (!(hi - lo > 0.0) || !std::isfinite(hi - lo)) {
        lo = 0.0;
        hi = 1.0;
        changed = true;
    }
    return changed;
}

}  // namespace

Viewport::Viewport(const PixelRect& area, double xMin, double xMax, double yMin, double yMax)
    : xMin_(xMin), xMax_(xMax), yMin_(yMin), yMax_(yMax) {
    area_.left = cleanCoord(area.left);
    area_.top = cleanCoord(area.top);
    area_.width = cleanLength(area.width);
    area_.height = cleanLength(area.height);
    const bool fixedX = repairRange(xMin_, xMax_);
    const bool fixedY = repairRange(yMin_, yMax_);
    adjusted_ = fixedX || fixedY;
}

double Viewport::mapX(double x) const {
    if (!std::isfinite(x)) return std::numeric_limits<double>::quiet_NaN();
    const double t = (x - xMin_) / (xMax_ - xMin_);
    const double v = area_.left + t * area_.width;
    if (std::isnan(v)) return area_.left;     // happens when width is 0 and t is infinite
    return clampPixel(v);
}

double Viewport::mapY(double y) const {
    if (!std::isfinite(y)) return std::numeric_limits<double>::quiet_NaN();
    const double t = (y - yMin_) / (yMax_ - yMin_);
    const double v = area_.bottom() - t * area_.height;
    if (std::isnan(v)) return area_.bottom();
    return clampPixel(v);
}

double Viewport::unmapX(double px) const {
    if (!std::isfinite(px)) return std::numeric_limits<double>::quiet_NaN();
    if (area_.width <= 0.0) return xMin_;
    const double t = (px - area_.left) / area_.width;
    return xMin_ + t * (xMax_ - xMin_);
}

double Viewport::unmapY(double py) const {
    if (!std::isfinite(py)) return std::numeric_limits<double>::quiet_NaN();
    if (area_.height <= 0.0) return yMin_;
    const double t = (area_.bottom() - py) / area_.height;
    return yMin_ + t * (yMax_ - yMin_);
}

void Viewport::applyClip(GraphicsContext& ctx) const {
    ctx.setClip(roundToInt(area_.left), roundToInt(area_.top),
                roundToInt(area_.right()), roundToInt(area_.bottom()));
}

}  // namespace gfx
