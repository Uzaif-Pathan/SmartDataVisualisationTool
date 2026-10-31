#pragma once
#include <optional>
#include <string>
#include <vector>

#include "graphics/Color.h"
#include "graphics/Renderer.h"

namespace gfx {

// One line in the chart. Points are connected in the order given (x is NOT
// auto-sorted, since re-ordering would silently change what a time series
// looks like). If `color` is not set, a palette colour is assigned by the
// series' position in LineChartData::series.
struct LineSeries {
    std::string name;
    std::vector<double> x;
    std::vector<double> y;
    std::optional<Color> color;
};

struct LineChartData {
    std::vector<LineSeries> series;
};

struct LineChartOptions {
    std::string title;
    std::string xAxisTitle;
    std::string yAxisTitle;

    bool showLegend = true;     // one entry per named series; hidden automatically if every name is empty
    bool showMarkers = true;    // small dot at each data point
    bool includeZeroY = false;  // force the y-axis to include 0 (off by default: line charts commonly zoom in)

    double lineWidth = 2.0;      // pixels; clamped to >= 1
    double markerRadius = 3.0;   // pixels; clamped to >= 0

    double titleSize = 20.0;
    double axisTitleSize = 14.0;
    double tickLabelSize = 12.0;
    double legendLabelSize = 12.0;

    Color plotBackground = colors::White;
    Color gridColor = Color(225, 225, 225);
    Color axisColor = colors::Black;
    Color zeroLineColor = Color(90, 90, 90);
    Color titleColor = colors::Black;
    Color axisTitleColor = colors::Black;
};

// Renders a complete line chart at (width x height) into `r`.
// x and y are both numeric axes (unlike BarChart/Histogram, neither is
// categorical or binned) — suited to a numeric or time-derived x value
// against a numeric y value.
//
// Handles, without crashing:
//  - 0 series, or a series with 0 points          (draws an empty frame, no crash)
//  - a NaN/infinite point                          (breaks the line at that point; valid
//                                                    points on either side still draw)
//  - a single point in a series                    (draws just its marker, no line segment)
//  - x and y arrays of different lengths            (uses the shorter length)
//  - unsorted x values                              (connects points in the given order,
//                                                    exactly as provided — not resorted)
//  - multiple series with very different ranges     (axes cover the combined range of all series)
//  - every point identical, including all 0         (axis widens automatically)
//  - very many points                               (renders without crashing)
//  - a tiny canvas                                  (falls back to ChartLayout's minimum plot area)
//  - NaN/negative/absurd option values              (each is sanitised before use)
//
// This function only renders. It never decides whether a line chart is the
// statistically appropriate choice for the data.
void drawLineChart(Renderer& r, double width, double height, const LineChartData& data,
                   const LineChartOptions& options = {});

}  // namespace gfx
