#pragma once
#include <string>
#include <vector>

#include "graphics/Color.h"
#include "graphics/Renderer.h"

namespace gfx {

// One slice: a category name and its (non-negative) share of the whole.
// This chart does not decide whether a pie chart is the statistically
// appropriate choice for the data (e.g. it will happily draw a pie of a
// continuous measurement if asked) — that judgement belongs to the
// recommendation module.
struct PieSlice {
    std::string label;
    double value = 0.0;
};

// Slices are drawn in the order given.
struct PieChartData {
    std::vector<PieSlice> slices;
};

struct PieChartOptions {
    std::string title;

    bool showLegend = true;          // one entry per slice, with its share as a percentage
    bool showPercentLabels = true;   // percentage drawn inside each large-enough slice
    double minLabelPercent = 0.03;   // inline label hidden below this share (0..1); the
                                     // slice and its legend entry are still drawn

    double startAngleDeg = 0.0;   // 0 = 12 o'clock; slices proceed from here
    bool clockwise = true;

    double titleSize = 20.0;
    double labelSize = 13.0;
    double legendLabelSize = 12.0;

    Color sliceStrokeColor = colors::White;  // thin border separating slices; matches
                                              // plotBackground to make slices blend instead
    int sliceStrokeWidth = 1;                // 0 disables the border

    Color plotBackground = colors::White;
    Color titleColor = colors::Black;

    // Text colour for inline percentage labels is chosen automatically per
    // slice (light text on a dark slice, dark text on a light one) so labels
    // stay readable across the whole palette; there is no override here.
};

// Renders a complete pie chart at (width x height) into `r`. Slices are
// coloured from the shared chart palette (charts/Legend.h), one colour per
// slice, in the order the slices were given (after invalid entries are
// removed — see below), the same convention BarChart/BoxPlot use for their
// categories.
//
// Handles, without crashing:
//  - 0 slices                                  (draws an empty circle outline, no crash)
//  - a NaN/infinite/negative value              (that slice is dropped entirely: no
//                                                wedge, no legend entry — a pie chart has
//                                                no natural zero-width slot to keep it in)
//  - every value 0, or no valid slices left      (draws an empty circle outline)
//  - exactly one valid slice                     (drawn as a full circle; percentage is
//                                                always 100%, avoiding degenerate 360-degree
//                                                wedge geometry)
//  - very many slices                            (renders without crashing; inline labels
//                                                thin out via minLabelPercent, legend rows
//                                                collapse to \"+N more\" if they overflow)
//  - a tiny canvas                               (radius shrinks accordingly; falls back to
//                                                ChartLayout's minimum plot area)
//  - NaN/negative/absurd option values           (each is sanitised before use)
void drawPieChart(Renderer& r, double width, double height, const PieChartData& data,
                  const PieChartOptions& options = {});

}  // namespace gfx
