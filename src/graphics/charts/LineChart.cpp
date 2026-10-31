#include "graphics/charts/LineChart.h"

#include <algorithm>
#include <cmath>
#include <limits>

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

double sanitizeNonNegative(double v, double fallback) {
    return (std::isfinite(v) && v >= 0.0) ? v : fallback;
}

}  // namespace

void drawLineChart(Renderer& r, double width, double height, const LineChartData& data,
                   const LineChartOptions& optionsIn) {
    LineChartOptions options = optionsIn;
    options.titleSize = sanitizeSize(options.titleSize, 20.0);
    options.axisTitleSize = sanitizeSize(options.axisTitleSize, 14.0);
    options.tickLabelSize = sanitizeSize(options.tickLabelSize, 12.0);
    options.legendLabelSize = sanitizeSize(options.legendLabelSize, 12.0);
    options.lineWidth = std::max(1.0, sanitizeNonNegative(options.lineWidth, 2.0));
    options.markerRadius = sanitizeNonNegative(options.markerRadius, 3.0);

    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0 || height <= 0.0) return;

    const std::size_t seriesCount = data.series.size();

    // --- combined x/y range across every series, from finite points only ---
    double xMin = std::numeric_limits<double>::infinity();
    double xMax = -std::numeric_limits<double>::infinity();
    double yMin = std::numeric_limits<double>::infinity();
    double yMax = -std::numeric_limits<double>::infinity();
    for (const LineSeries& s : data.series) {
        const std::size_t n = std::min(s.x.size(), s.y.size());
        for (std::size_t i = 0; i < n; ++i) {
            if (!std::isfinite(s.x[i]) || !std::isfinite(s.y[i])) continue;
            xMin = std::min(xMin, s.x[i]);
            xMax = std::max(xMax, s.x[i]);
            yMin = std::min(yMin, s.y[i]);
            yMax = std::max(yMax, s.y[i]);
        }
    }
    if (!std::isfinite(xMin) || !std::isfinite(xMax)) { xMin = 0.0; xMax = 1.0; }
    if (!std::isfinite(yMin) || !std::isfinite(yMax)) { yMin = 0.0; yMax = 1.0; }

    const AxisScale xScale = niceScale(xMin, xMax, 6, false);
    const AxisScale yScale = niceScale(yMin, yMax, 6, options.includeZeroY);

    // --- legend entries: one per named series, colour-matched to its line ---
    std::vector<LegendEntry> legendEntries;
    bool anyNamed = false;
    for (const LineSeries& s : data.series)
        if (!s.name.empty()) anyNamed = true;
    if (options.showLegend && anyNamed) {
        for (std::size_t i = 0; i < seriesCount; ++i) {
            if (data.series[i].name.empty()) continue;
            const Color c = data.series[i].color.value_or(paletteColor(i));
            legendEntries.push_back(LegendEntry{data.series[i].name, c});
        }
    }

    // --- widest y tick label, so ChartLayout leaves enough room on the left ---
    FontManager& font = FontManager::defaultFont();
    double widestYLabel = 0.0;
    for (const std::string& label : tickLabels(yScale))
        widestYLabel = std::max(widestYLabel, measureText(font, label, options.tickLabelSize).width);

    const double legendWidth = !legendEntries.empty()
                                    ? measureLegendWidth(legendEntries, options.legendLabelSize)
                                    : 0.0;

    LayoutOptions layoutOpts;
    layoutOpts.hasTitle = !options.title.empty();
    layoutOpts.hasXAxisTitle = !options.xAxisTitle.empty();
    layoutOpts.hasYAxisTitle = !options.yAxisTitle.empty();
    layoutOpts.hasLegend = !legendEntries.empty();
    layoutOpts.titleSize = options.titleSize;
    layoutOpts.axisTitleSize = options.axisTitleSize;
    layoutOpts.tickLabelSize = options.tickLabelSize;
    layoutOpts.yTickLabelWidth = widestYLabel;
    layoutOpts.legendWidth = legendWidth;

    const ChartLayout layout = computeChartLayout(width, height, layoutOpts);
    const PixelRect& plotArea = layout.plotArea;

    const Viewport vp(plotArea, xScale.min, xScale.max, yScale.min, yScale.max);

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

    // --- grid, drawn before the lines so lines sit on top ---
    drawHorizontalGrid(r, vp, yScale.ticks, options.gridColor);
    drawVerticalGrid(r, vp, xScale.ticks, options.gridColor);
    drawZeroLines(r, vp, options.zeroLineColor);

    // --- lines and markers, clipped to the plot area ---
    if (plotArea.width > 0.0 && plotArea.height > 0.0) {
        r.context().setClip(roundToInt(plotArea.left), roundToInt(plotArea.top),
                            roundToInt(plotArea.right()), roundToInt(plotArea.bottom()));
        for (std::size_t s = 0; s < seriesCount; ++s) {
            const LineSeries& series = data.series[s];
            const std::size_t n = std::min(series.x.size(), series.y.size());
            const Color color = series.color.value_or(paletteColor(s));

            // Segments: skip any segment touching a non-finite point, so the
            // line breaks there instead of jumping across a gap.
            for (std::size_t i = 0; i + 1 < n; ++i) {
                if (!std::isfinite(series.x[i]) || !std::isfinite(series.y[i]) ||
                    !std::isfinite(series.x[i + 1]) || !std::isfinite(series.y[i + 1]))
                    continue;
                Line seg;
                seg.x0 = vp.mapX(series.x[i]);
                seg.y0 = vp.mapY(series.y[i]);
                seg.x1 = vp.mapX(series.x[i + 1]);
                seg.y1 = vp.mapY(series.y[i + 1]);
                seg.color = color;
                seg.thickness = static_cast<int>(std::lround(options.lineWidth));
                r.draw(seg);
            }
            // Markers, drawn after all segments so they sit on top of every line.
            if (options.showMarkers && options.markerRadius > 0.0) {
                for (std::size_t i = 0; i < n; ++i) {
                    if (!std::isfinite(series.x[i]) || !std::isfinite(series.y[i])) continue;
                    Circle marker;
                    marker.cx = vp.mapX(series.x[i]);
                    marker.cy = vp.mapY(series.y[i]);
                    marker.radius = options.markerRadius;
                    marker.fill = color;
                    r.draw(marker);
                }
            }
        }
        r.context().clearClip();
    }

    // --- frame and ticks ---
    const AxisStyle axisStyle{options.axisColor, Color(40, 40, 40), options.tickLabelSize};
    drawPlotFrame(r, plotArea, axisStyle);
    drawXAxisTicks(r, vp, xScale, layout, axisStyle);
    drawYAxisTicks(r, vp, yScale, layout, axisStyle);

    // --- legend ---
    if (layoutOpts.hasLegend)
        drawLegend(r, layout.legendArea, legendEntries, options.legendLabelSize, Color(40, 40, 40));

    // --- titles ---
    drawTitle(r, layout, options.title, options.titleSize, options.titleColor);
    drawXAxisTitle(r, layout, options.xAxisTitle, options.axisTitleSize, options.axisTitleColor);
    drawYAxisTitle(r, layout, options.yAxisTitle, options.axisTitleSize, options.axisTitleColor);
}

}  // namespace gfx
