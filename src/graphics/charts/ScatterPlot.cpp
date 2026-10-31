#include "graphics/charts/ScatterPlot.h"

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
#include "graphics/primitives/Rectangle.h"
#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

namespace gfx {
namespace {

double sanitizeSize(double v, double fallback) {
    return (std::isfinite(v) && v > 0.0) ? v : fallback;
}

double clamp01(double v, double fallback) {
    if (!std::isfinite(v)) return fallback;
    return std::max(0.0, std::min(1.0, v));
}

}  // namespace

void drawScatterPlot(Renderer& r, double width, double height, const ScatterPlotData& data,
                     const ScatterPlotOptions& optionsIn) {
    ScatterPlotOptions options = optionsIn;
    options.titleSize = sanitizeSize(options.titleSize, 20.0);
    options.axisTitleSize = sanitizeSize(options.axisTitleSize, 14.0);
    options.tickLabelSize = sanitizeSize(options.tickLabelSize, 12.0);
    options.legendLabelSize = sanitizeSize(options.legendLabelSize, 12.0);
    options.pointRadius = std::max(0.5, (std::isfinite(options.pointRadius) ? options.pointRadius : 4.0));
    options.pointOpacity = clamp01(options.pointOpacity, 0.75);

    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0 || height <= 0.0) return;

    const std::size_t seriesCount = data.series.size();

    // --- combined x/y range across every series, from finite points only ---
    double xMin = std::numeric_limits<double>::infinity();
    double xMax = -std::numeric_limits<double>::infinity();
    double yMin = std::numeric_limits<double>::infinity();
    double yMax = -std::numeric_limits<double>::infinity();
    for (const ScatterSeries& s : data.series) {
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

    const AxisScale xScale = niceScale(xMin, xMax, 6, options.includeZeroX);
    const AxisScale yScale = niceScale(yMin, yMax, 6, options.includeZeroY);

    // --- legend entries: one per named series, colour-matched to its points ---
    std::vector<LegendEntry> legendEntries;
    bool anyNamed = false;
    for (const ScatterSeries& s : data.series)
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

    // --- grid, drawn before the points so points sit on top ---
    drawHorizontalGrid(r, vp, yScale.ticks, options.gridColor);
    drawVerticalGrid(r, vp, xScale.ticks, options.gridColor);
    drawZeroLines(r, vp, options.zeroLineColor);

    // --- points, clipped to the plot area ---
    if (plotArea.width > 0.0 && plotArea.height > 0.0) {
        r.context().setClip(roundToInt(plotArea.left), roundToInt(plotArea.top),
                            roundToInt(plotArea.right()), roundToInt(plotArea.bottom()));
        const std::uint8_t alpha = static_cast<std::uint8_t>(std::lround(options.pointOpacity * 255.0));
        for (std::size_t s = 0; s < seriesCount; ++s) {
            const ScatterSeries& series = data.series[s];
            const std::size_t n = std::min(series.x.size(), series.y.size());
            const Color base = series.color.value_or(paletteColor(s));
            const Color drawColor(base.r, base.g, base.b, alpha);

            for (std::size_t i = 0; i < n; ++i) {
                if (!std::isfinite(series.x[i]) || !std::isfinite(series.y[i])) continue;
                const double px = vp.mapX(series.x[i]);
                const double py = vp.mapY(series.y[i]);
                if (!std::isfinite(px) || !std::isfinite(py)) continue;

                Circle point;
                point.cx = px;
                point.cy = py;
                point.radius = options.pointRadius;
                point.fill = drawColor;
                r.draw(point);
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
