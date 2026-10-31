#pragma once
#include <vector>

#include "graphics/Color.h"
#include "graphics/Renderer.h"
#include "graphics/Viewport.h"

namespace gfx {

// Horizontal lines across the plot area at the given DATA y values
// (usually AxisScale::ticks).  Values that map outside the plot area, or to
// NaN, are skipped.
void drawHorizontalGrid(Renderer& r, const Viewport& vp, const std::vector<double>& yTicks,
                        const Color& color);

// Vertical lines at the given DATA x values.
void drawVerticalGrid(Renderer& r, const Viewport& vp, const std::vector<double>& xTicks,
                      const Color& color);

// A stronger line where y = 0 and/or x = 0, drawn only when 0 lies strictly
// inside the axis range (so it never doubles up with the frame).
void drawZeroLines(Renderer& r, const Viewport& vp, const Color& color);

}  // namespace gfx
