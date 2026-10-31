#pragma once
#include <string>
#include <vector>

#include "graphics/Color.h"
#include "graphics/Renderer.h"

namespace gfx {

// One bar chart: a set of named categories, each with a single numeric value.
// `categories` and `values` should be the same length; if they differ, the
// shorter length wins and the extra entries on the longer side are ignored.
// A non-finite (NaN/infinite) value draws no bar for that category, but the
// category still gets its axis label and legend entry.
struct BarChartData {
    std::vector<std::string> categories;
    std::vector<double> values;
};

struct BarChartOptions {
    std::string title;
    std::string xAxisTitle;
    std::string yAxisTitle;

    bool showLegend = true;    // one legend entry per category, colour-matched to its bar
    bool includeZero = true;   // keep the y-axis baseline at 0 (recommended for bar charts)

    double titleSize = 20.0;
    double axisTitleSize = 14.0;
    double tickLabelSize = 12.0;
    double categoryLabelSize = 12.0;
    double legendLabelSize = 12.0;

    double barGapRatio = 0.25;  // gap between bars, as a fraction of the slot width; clamped to 0..0.9

    Color plotBackground = colors::White;
    Color gridColor = Color(225, 225, 225);
    Color axisColor = colors::Black;
    Color zeroLineColor = Color(90, 90, 90);
    Color titleColor = colors::Black;
    Color axisTitleColor = colors::Black;
};

// Renders a complete bar chart at (width x height) into `r`.
// Bars are coloured from the shared chart palette (charts/Legend.h), one
// colour per category, in the order the categories were given.
//
// Handles, without crashing:
//  - 0 categories                        (draws an empty frame, no crash)
//  - a NaN or infinite value              (skipped: no bar, but label/legend stay)
//  - every value equal, including all 0   (axis widens automatically)
//  - negative values                      (bars grow downward from the 0 line)
//  - categories/values length mismatch    (uses the shorter length)
//  - very many categories                 (category labels thin out; bars can
//                                           get very thin but never overlap or vanish)
//  - a tiny canvas                        (falls back to ChartLayout's minimum plot area)
//  - NaN/negative/absurd option values    (each is sanitised before use)
void drawBarChart(Renderer& r, double width, double height, const BarChartData& data,
                  const BarChartOptions& options = {});

}  // namespace gfx
