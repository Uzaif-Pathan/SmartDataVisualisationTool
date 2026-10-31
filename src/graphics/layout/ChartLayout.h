#pragma once
#include "graphics/Viewport.h"

namespace gfx {

// What the chart contains.  Sizes are in pixels.
struct LayoutOptions {
    bool hasTitle = false;
    bool hasXAxisTitle = false;
    bool hasYAxisTitle = false;
    bool hasLegend = false;
    double titleSize = 20.0;
    double axisTitleSize = 14.0;
    double tickLabelSize = 12.0;
    double yTickLabelWidth = 0.0;   // width of the WIDEST y tick label: measure it
                                    // with measureText() so the layout can leave room
    double legendWidth = 0.0;       // used only when hasLegend is true
};

// Where everything goes.  The anchors are meant for Text like this:
//   title    -> HAlign::Center, VAlign::Top      at (titleX,  titleY)
//   x title  -> HAlign::Center, VAlign::Bottom   at (xTitleX, xTitleY)
//   y title  -> Center + Middle, TextRotation::Up at (yTitleX, yTitleY)
struct ChartLayout {
    PixelRect plotArea;       // where the data is drawn (feed this to Viewport)
    PixelRect legendArea;     // empty (all zeros) when there is no legend
    double titleX = 0.0;
    double titleY = 0.0;
    double xTitleX = 0.0;
    double xTitleY = 0.0;
    double yTitleX = 0.0;
    double yTitleY = 0.0;
    double tickLength = 4.0;      // length of the little tick marks
    double tickLabelGap = 3.0;    // space between a tick mark and its label
    double padding = 8.0;         // outer padding around the whole chart
};

// Splits a width x height canvas into plot area, title area, axis-title areas
// and legend area.  Margins scale with the canvas size.  On very small canvases
// the margins are squeezed so the plot area stays at least ~10 px wide/tall
// (or fills the canvas if the canvas itself is smaller than that).
// Invalid size (<= 0, NaN, infinite) returns an all-zero layout.
ChartLayout computeChartLayout(double width, double height, const LayoutOptions& options);

}  // namespace gfx
