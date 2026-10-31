#include "graphics/charts/BoxPlot.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <optional>

#include "graphics/Viewport.h"
#include "graphics/charts/Axis.h"
#include "graphics/charts/Grid.h"
#include "graphics/charts/Legend.h"
#include "graphics/charts/Title.h"
#include "graphics/layout/AxisLayout.h"
#include "graphics/layout/ChartLayout.h"
#include "graphics/primitives/Circle.h"
#include "graphics/primitives/Line.h"
#include "graphics/primitives/Rectangle.h"
#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

namespace gfx {
namespace {

double sanitizeSize(double v, double fallback) {
    return (std::isfinite(v) && v > 0.0) ? v : fallback;
}

double clampRatio(double v, double lo, double hi, double fallback) {
    if (!std::isfinite(v)) return fallback;
    return std::max(lo, std::min(hi, v));
}

// Returns the five key stats sorted ascending (whiskerLow..whiskerHigh), or
// nullopt if any of them is NaN/infinite. A malformed order (e.g. median
// given below q1) is repaired by sorting rather than trusted, so the box
// always draws something geometrically sane.
std::optional<std::array<double, 5>> sanitizedFive(const BoxPlotStats& s) {
    std::array<double, 5> v{s.whiskerLow, s.q1, s.median, s.q3, s.whiskerHigh};
    for (double x : v)
        if (!std::isfinite(x)) return std::nullopt;
    std::sort(v.begin(), v.end());
    return v;
}

}  // namespace

void drawBoxPlot(Renderer& r, double width, double height, const BoxPlotData& data,
                 const BoxPlotOptions& optionsIn) {
    BoxPlotOptions options = optionsIn;
    options.titleSize = sanitizeSize(options.titleSize, 20.0);
    options.axisTitleSize = sanitizeSize(options.axisTitleSize, 14.0);
    options.tickLabelSize = sanitizeSize(options.tickLabelSize, 12.0);
    options.categoryLabelSize = sanitizeSize(options.categoryLabelSize, 12.0);
    options.boxWidthRatio = clampRatio(options.boxWidthRatio, 0.05, 0.9, 0.5);
    options.capWidthRatio = clampRatio(options.capWidthRatio, 0.0, 1.0, 0.4);
    options.outlierRadius = std::isfinite(options.outlierRadius)
                                ? std::max(0.5, options.outlierRadius)
                                : 3.0;

    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0 || height <= 0.0) return;

    const std::size_t n = data.boxes.size();

    // Pre-sanitize every box once: sorted five-number summary (or nullopt if
    // unusable) plus the finite subset of its outliers. Reused below both for
    // the axis range and for drawing.
    std::vector<std::optional<std::array<double, 5>>> fives(n);
    std::vector<std::vector<double>> finiteOutliers(n);
    for (std::size_t i = 0; i < n; ++i) {
        fives[i] = sanitizedFive(data.boxes[i].stats);
        finiteOutliers[i].reserve(data.boxes[i].stats.outliers.size());
        for (double v : data.boxes[i].stats.outliers)
            if (std::isfinite(v)) finiteOutliers[i].push_back(v);
    }

    // --- y-axis range, from every finite whisker end and outlier ---
    double dataMin = std::numeric_limits<double>::infinity();
    double dataMax = -std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < n; ++i) {
        if (fives[i]) {
            dataMin = std::min(dataMin, (*fives[i])[0]);
            dataMax = std::max(dataMax, (*fives[i])[4]);
        }
        for (double v : finiteOutliers[i]) {
            dataMin = std::min(dataMin, v);
            dataMax = std::max(dataMax, v);
        }
    }
    if (!std::isfinite(dataMin) || !std::isfinite(dataMax)) {
        dataMin = 0.0;
        dataMax = 1.0;
    }
    const AxisScale yScale = niceScale(dataMin, dataMax, 6, options.includeZero);

    // --- widest y tick label, so ChartLayout leaves enough room on the left ---
    FontManager& font = FontManager::defaultFont();
    double widestYLabel = 0.0;
    for (const std::string& label : tickLabels(yScale))
        widestYLabel = std::max(widestYLabel, measureText(font, label, options.tickLabelSize).width);

    LayoutOptions layoutOpts;
    layoutOpts.hasTitle = !options.title.empty();
    layoutOpts.hasXAxisTitle = !options.xAxisTitle.empty();
    layoutOpts.hasYAxisTitle = !options.yAxisTitle.empty();
    layoutOpts.hasLegend = false;  // categories are already labelled on the x-axis
    layoutOpts.titleSize = options.titleSize;
    layoutOpts.axisTitleSize = options.axisTitleSize;
    layoutOpts.tickLabelSize = options.tickLabelSize;
    layoutOpts.yTickLabelWidth = widestYLabel;

    const ChartLayout layout = computeChartLayout(width, height, layoutOpts);
    const PixelRect& plotArea = layout.plotArea;

    // x domain is purely categorical: [0, n]. Category i occupies [i, i+1).
    const Viewport vp(plotArea, 0.0, n > 0 ? static_cast<double>(n) : 1.0, yScale.min, yScale.max);

    // --- plot background ---
    if (plotArea.width > 0.0 && plotArea.height > 0.0) {
        Rectangle bg;
        bg.x = plotArea.left;
        bg.y = plotArea.top;
        bg.width = plotArea.width;
        bg.height = plotArea.height;
        bg.fill = options.plotBackground;
        r.draw(bg);
    }

    // --- grid, drawn before the boxes so boxes sit on top ---
    drawHorizontalGrid(r, vp, yScale.ticks, options.gridColor);
    drawZeroLines(r, vp, options.zeroLineColor);

    // --- boxes, clipped to the plot area ---
    const double slotWidth = n > 0 ? plotArea.width / static_cast<double>(n) : 0.0;
    const double boxWidth = std::max(1.0, slotWidth * options.boxWidthRatio);
    const double capHalfWidth = std::max(1.0, boxWidth * options.capWidthRatio) / 2.0;

    std::vector<double> categoryCentersX(n);
    std::vector<std::string> categories(n);
    for (std::size_t i = 0; i < n; ++i) {
        categoryCentersX[i] = vp.mapX(static_cast<double>(i) + 0.5);
        categories[i] = data.boxes[i].category;
    }

    if (plotArea.width > 0.0 && plotArea.height > 0.0) {
        r.context().setClip(roundToInt(plotArea.left), roundToInt(plotArea.top),
                            roundToInt(plotArea.right()), roundToInt(plotArea.bottom()));

        for (std::size_t i = 0; i < n; ++i) {
            const double cx = categoryCentersX[i];
            const Color color = paletteColor(i);

            if (fives[i]) {
                const std::array<double, 5>& v = *fives[i];
                const double yLow = vp.mapY(v[0]);
                const double yQ1 = vp.mapY(v[1]);
                const double yMedian = vp.mapY(v[2]);
                const double yQ3 = vp.mapY(v[3]);
                const double yHigh = vp.mapY(v[4]);

                if (std::isfinite(yLow) && std::isfinite(yQ1) && std::isfinite(yMedian) &&
                    std::isfinite(yQ3) && std::isfinite(yHigh)) {
                    // Whiskers: lower whisker line, upper whisker line.
                    r.draw(Line{cx, yLow, cx, yQ1, color, 1});
                    r.draw(Line{cx, yQ3, cx, yHigh, color, 1});

                    // Caps at each whisker end.
                    r.draw(Line{cx - capHalfWidth, yLow, cx + capHalfWidth, yLow, color, 1});
                    r.draw(Line{cx - capHalfWidth, yHigh, cx + capHalfWidth, yHigh, color, 1});

                    // Box from Q1 to Q3 (top is the smaller pixel y, i.e. Q3).
                    Rectangle box;
                    box.x = cx - boxWidth / 2.0;
                    box.y = std::min(yQ1, yQ3);
                    box.width = boxWidth;
                    box.height = std::max(1.0, std::fabs(yQ1 - yQ3));
                    box.fill = Color(color.r, color.g, color.b, 140);
                    box.stroke = color;
                    box.strokeWidth = 1;
                    r.draw(box);

                    // Median line, drawn on top of the box.
                    r.draw(Line{cx - boxWidth / 2.0, yMedian, cx + boxWidth / 2.0, yMedian,
                               options.medianColor, 2});
                }
            }

            // Outliers: hollow dots beyond the whiskers, drawn even when the
            // box itself could not be drawn (so a single bad summary never
            // hides otherwise-valid outlier points).
            for (double value : finiteOutliers[i]) {
                const double y = vp.mapY(value);
                if (!std::isfinite(y)) continue;
                Circle dot;
                dot.cx = cx;
                dot.cy = y;
                dot.radius = options.outlierRadius;
                dot.stroke = color;
                dot.strokeWidth = 1;
                r.draw(dot);
            }
        }

        r.context().clearClip();
    }

    // --- frame, ticks, category labels ---
    const AxisStyle axisStyle{options.axisColor, Color(40, 40, 40), options.tickLabelSize};
    drawPlotFrame(r, plotArea, axisStyle);
    drawYAxisTicks(r, vp, yScale, layout, axisStyle);

    const AxisStyle categoryStyle{options.axisColor, Color(40, 40, 40), options.categoryLabelSize};
    drawCategoryLabels(r, categoryCentersX, categories, slotWidth, plotArea, layout, categoryStyle);

    // --- titles ---
    drawTitle(r, layout, options.title, options.titleSize, options.titleColor);
    drawXAxisTitle(r, layout, options.xAxisTitle, options.axisTitleSize, options.axisTitleColor);
    drawYAxisTitle(r, layout, options.yAxisTitle, options.axisTitleSize, options.axisTitleColor);
}

}  // namespace gfx
