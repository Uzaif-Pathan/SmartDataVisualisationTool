#pragma once
#include <string>
#include <vector>

#include "graphics/Color.h"
#include "graphics/Renderer.h"

namespace gfx {

// One histogram: a set of raw numeric observations. Binning is done inside
// drawHistogram (Sturges' rule by default), since no other module in this
// project currently owns histogram bin selection. If Member 1 later adds a
// dedicated bin calculator, pass its bin count in via HistogramOptions::binCount
// and this module will use that instead of computing its own.
struct HistogramData {
    std::vector<double> values;   // raw observations; NaN/infinite entries are ignored
};

struct HistogramOptions {
    std::string title;
    std::string xAxisTitle;
    std::string yAxisTitle = "Frequency";

    // 0 = choose automatically with Sturges' rule: ceil(log2(n) + 1), minimum 1.
    // Any positive value overrides the automatic choice.
    int binCount = 0;

    double titleSize = 20.0;
    double axisTitleSize = 14.0;
    double tickLabelSize = 12.0;

    // Small gap between bars, as a fraction of bin width, so adjacent bins
    // are visually distinct. Histograms conventionally sit near 0 (unlike bar
    // charts). Clamped to 0..0.5.
    double barGapRatio = 0.02;

    Color barColor = Color(70, 130, 200);
    Color plotBackground = colors::White;
    Color gridColor = Color(225, 225, 225);
    Color axisColor = colors::Black;
    Color zeroLineColor = Color(90, 90, 90);
    Color titleColor = colors::Black;
    Color axisTitleColor = colors::Black;
};

// Renders a complete histogram at (width x height) into `r`.
//
// Handles, without crashing:
//  - 0 values, or all values NaN/infinite    (draws an empty frame, no crash)
//  - every value identical, including all 0  (single bin, axis widened automatically)
//  - negative values                          (x-axis extends below 0 as needed)
//  - a single observation                     (single bin)
//  - very many values / very many bins        (thins bin-edge labels; bars never vanish)
//  - a tiny canvas                             (falls back to ChartLayout's minimum plot area)
//  - NaN/negative/absurd option values         (each is sanitised before use)
//
// This function only renders. It never decides whether a histogram is the
// statistically appropriate chart for the data — that decision belongs to
// the recommendation/visualization-selection module.
void drawHistogram(Renderer& r, double width, double height, const HistogramData& data,
                   const HistogramOptions& options = {});

}  // namespace gfx
