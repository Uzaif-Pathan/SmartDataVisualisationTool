#pragma once
#include <string>
#include <vector>

#include "graphics/Color.h"
#include "graphics/Renderer.h"
#include "graphics/Viewport.h"
#include "graphics/layout/AxisLayout.h"
#include "graphics/layout/ChartLayout.h"

namespace gfx {

struct AxisStyle {
    Color lineColor = colors::Black;        // frame and tick marks
    Color labelColor = Color(40, 40, 40);   // tick label text
    double labelSize = 12.0;                // tick label font size in pixels
};

// Draws the rectangle around the plot area.  Does nothing for an empty area.
void drawPlotFrame(Renderer& r, const PixelRect& plotArea, const AxisStyle& style);

// Tick marks and numbers along the bottom edge of the plot area.
// Every tick gets a mark.  Labels are thinned out (every 2nd, 3rd, ...) when
// they would touch each other.  Ticks that map to NaN are skipped.
void drawXAxisTicks(Renderer& r, const Viewport& vp, const AxisScale& scale,
                    const ChartLayout& layout, const AxisStyle& style);

// Tick marks and numbers along the left edge.  Labels are right-aligned and
// vertically centred on their tick.
void drawYAxisTicks(Renderer& r, const Viewport& vp, const AxisScale& scale,
                    const ChartLayout& layout, const AxisStyle& style);

// Labels under categories (bar charts, box plots, ...).
//  - centersX[i] is the pixel x of the middle of category i.
//  - slotWidth is the pixel width available to one category.
// Long names are shortened with "...".  When slots are very narrow, only
// every 2nd, 3rd, ... label is drawn.  If the two lists differ in length, the
// shorter one decides how many labels are drawn.
void drawCategoryLabels(Renderer& r, const std::vector<double>& centersX,
                        const std::vector<std::string>& labels, double slotWidth,
                        const PixelRect& plotArea, const ChartLayout& layout,
                        const AxisStyle& style);

}  // namespace gfx
