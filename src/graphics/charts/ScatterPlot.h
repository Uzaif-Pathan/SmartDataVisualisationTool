#pragma once
#include <optional>
#include <string>
#include <vector>

#include "graphics/Color.h"
#include "graphics/Renderer.h"

namespace gfx {

// One group of points. Unlike LineSeries, points are NOT connected — a
// scatter plot shows the relationship between two numeric variables, not a
// sequence. If `color` is not set, a palette colour is assigned by the
// series' position in ScatterPlotData::series.
struct ScatterSeries {
    std::string name;
    std::vector<double> x;
    std::vector<double> y;
    std::optional<Color> color;
};

struct ScatterPlotData {
    std::vector<ScatterSeries> series;
};

struct ScatterPlotOptions {
    std::string title;
    std::string xAxisTitle;
    std::string yAxisTitle;

    bool showLegend = true;      // one entry per named series; hidden automatically if every name is empty
    bool includeZeroX = false;   // force the x-axis to include 0
    bool includeZeroY = false;   // force the y-axis to include 0

    double pointRadius = 4.0;    // pixels; clamped to >= 0.5
    double pointOpacity = 0.75;  // 0..1; slight transparency helps show overlapping points

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

// Renders a complete scatter plot at (width x height) into `r`. Both axes are
// numeric. Points are drawn only — this function never computes or draws a
// trend/best-fit line and never decides whether a scatter plot is the
// statistically appropriate choice for the data (that belongs to the
// recommendation/visualization-selection module).
//
// Handles, without crashing:
//  - 0 series, or a series with 0 points          (draws an empty frame, no crash)
//  - a NaN/infinite point                          (that point is skipped; others still draw)
//  - x and y arrays of different lengths            (uses the shorter length)
//  - multiple series with very different ranges     (axes cover the combined range of all series)
//  - every point identical, including all 0         (axis widens automatically)
//  - very many points                               (renders without crashing)
//  - a tiny canvas                                  (falls back to ChartLayout's minimum plot area)
//  - NaN/negative/absurd option values              (each is sanitised before use)
void drawScatterPlot(Renderer& r, double width, double height, const ScatterPlotData& data,
                     const ScatterPlotOptions& options = {});

}  // namespace gfx
