#pragma once
#include "graphics/GraphicsContext.h"

namespace gfx {

// A rectangle in PIXEL coordinates: (left, top) is the top-left corner,
// y grows downward.
struct PixelRect {
    double left = 0.0;
    double top = 0.0;
    double width = 0.0;
    double height = 0.0;

    double right() const { return left + width; }
    double bottom() const { return top + height; }
    double centerX() const { return left + width / 2.0; }
    double centerY() const { return top + height / 2.0; }
};

// Maps DATA coordinates to PIXEL coordinates inside a plot area.
//
//   pixelX = left   + (x - xMin) / (xMax - xMin) * width
//   pixelY = bottom - (y - yMin) / (yMax - yMin) * height      (y is flipped)
//
// So (xMin, yMin) lands on the bottom-left corner of the area and
// (xMax, yMax) on the top-right corner.
//
// Bad input is repaired rather than rejected:
//  - min > max          -> swapped
//  - min == max         -> range widened by 10% of the value (or +/-1 for 0)
//  - NaN / infinite     -> range replaced by 0..1
//  - bad area size      -> treated as 0 (everything maps to the top-left)
// wasAdjusted() tells you if any repair happened.
//
// mapX/mapY of a NaN or infinite value return NaN (shapes drawn there draw
// nothing).  Finite results are limited to +/- 10,000,000 pixels.
class Viewport {
public:
    Viewport(const PixelRect& area, double xMin, double xMax, double yMin, double yMax);

    const PixelRect& area() const { return area_; }
    double xMin() const { return xMin_; }
    double xMax() const { return xMax_; }
    double yMin() const { return yMin_; }
    double yMax() const { return yMax_; }
    bool wasAdjusted() const { return adjusted_; }

    double mapX(double x) const;      // data -> pixel
    double mapY(double y) const;
    double unmapX(double px) const;   // pixel -> data (for hover/click later)
    double unmapY(double py) const;

    // Restricts drawing to the plot area, so data outside the axis
    // range cannot spill onto the axes, labels or title.
    void applyClip(GraphicsContext& ctx) const;

private:
    PixelRect area_;
    double xMin_;
    double xMax_;
    double yMin_;
    double yMax_;
    bool adjusted_ = false;
};

}  // namespace gfx
