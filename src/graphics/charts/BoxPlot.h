#pragma once
#include <string>
#include <vector>

#include "graphics/Color.h"
#include "graphics/Renderer.h"

namespace gfx {

// The five-number summary for one box, plus outliers. This chart does NOT
// compute quartiles, whiskers, or outlier detection itself — that belongs to
// the statistics module. Pass in numbers that are already decided:
//   whiskerLow / whiskerHigh  -> usually the most extreme non-outlier values
//                                (e.g. the classic Q1 - 1.5*IQR / Q3 + 1.5*IQR
//                                rule), NOT the absolute min/max of the data
//   outliers                  -> individual points beyond the whiskers,
//                                drawn as separate dots so they are never
//                                silently hidden inside the box
// If whiskerLow/q1/median/q3/whiskerHigh are not already in ascending order,
// they are sorted before drawing (so a mistaken order never crashes or draws
// something misleading) — but the caller should not rely on that; supply
// them correctly ordered.
struct BoxPlotStats {
    double whiskerLow = 0.0;
    double q1 = 0.0;
    double median = 0.0;
    double q3 = 0.0;
    double whiskerHigh = 0.0;
    std::vector<double> outliers;
};

struct BoxPlotEntry {
    std::string category;
    BoxPlotStats stats;
};

// One box per category, in the order given.
struct BoxPlotData {
    std::vector<BoxPlotEntry> boxes;
};

struct BoxPlotOptions {
    std::string title;
    std::string xAxisTitle;
    std::string yAxisTitle;

    bool includeZero = false;   // force the y-axis to include 0

    double boxWidthRatio = 0.5;  // box width as a fraction of the slot width; clamped to 0.05..0.9
    double capWidthRatio = 0.4;  // whisker cap width as a fraction of the box width; clamped to 0..1
    double outlierRadius = 3.0;  // pixels; clamped to >= 0.5

    double titleSize = 20.0;
    double axisTitleSize = 14.0;
    double tickLabelSize = 12.0;
    double categoryLabelSize = 12.0;

    Color plotBackground = colors::White;
    Color gridColor = Color(225, 225, 225);
    Color axisColor = colors::Black;
    Color zeroLineColor = Color(90, 90, 90);
    Color medianColor = Color(30, 30, 30);
    Color titleColor = colors::Black;
    Color axisTitleColor = colors::Black;
};

// Renders a complete box plot at (width x height) into `r`. Boxes are
// coloured from the shared chart palette (charts/Legend.h), one colour per
// category, in the order the categories were given. The renderer never
// decides whether a box plot is the statistically appropriate choice for the
// data, and never computes the five-number summary itself — both belong to
// the recommendation/statistics modules.
//
// Handles, without crashing:
//  - 0 categories                          (draws an empty frame, no crash)
//  - a box with a NaN/infinite stat         (that box is skipped: no box or
//                                            whiskers, but label stays; its
//                                            outliers are still checked
//                                            individually)
//  - a NaN/infinite outlier value           (that one point is skipped)
//  - whiskerLow/q1/median/q3/whiskerHigh
//    given out of order                     (sorted before drawing)
//  - a box with zero-width IQR (q1 == q3)   (box still draws, at minimum 1px)
//  - many categories                        (category labels thin out, same
//                                            as bar charts)
//  - a tiny canvas                          (falls back to ChartLayout's
//                                            minimum plot area)
//  - NaN/negative/absurd option values      (each is sanitised before use)
void drawBoxPlot(Renderer& r, double width, double height, const BoxPlotData& data,
                 const BoxPlotOptions& options = {});

}  // namespace gfx
