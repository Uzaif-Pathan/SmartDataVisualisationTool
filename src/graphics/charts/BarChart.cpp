#include "graphics/charts/BarChart.h"

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
#include "graphics/primitives/Rectangle.h"
#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

namespace gfx {
namespace {

double sanitizeSize(double v, double fallback) {
    return (std::isfinite(v) && v > 0.0) ? v : fallback;
}

double clamp01to09(double v) {
    if (!std::isfinite(v)) return 0.25;
    return std::max(0.0, std::min(0.9, v));
}

// Baseline the bars grow from: 0 if the axis actually spans 0, otherwise the
// axis minimum (so a bar is never drawn "backwards" outside the axis).
double chooseBaseline(const AxisScale& scale) {
    if (scale.min <= 0.0 && scale.max >= 0.0) return 0.0;
    return scale.min;
}

}  // namespace

void drawBarChart(Renderer& r, double width, double height, const BarChartData& data,
                  const BarChartOptions& optionsIn) {
    BarChartOptions options = optionsIn;
    options.titleSize = sanitizeSize(options.titleSize, 20.0);
    options.axisTitleSize = sanitizeSize(options.axisTitleSize, 14.0);
    options.tickLabelSize = sanitizeSize(options.tickLabelSize, 12.0);
    options.categoryLabelSize = sanitizeSize(options.categoryLabelSize, 12.0);
    options.legendLabelSize = sanitizeSize(options.legendLabelSize, 12.0);
    options.barGapRatio = clamp01to09(options.barGapRatio);

    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0 || height <= 0.0) return;

    const std::size_t n = std::min(data.categories.size(), data.values.size());

    // --- y-axis range, from the finite values only ---
    double dataMin = std::numeric_limits<double>::infinity();
    double dataMax = -std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < n; ++i) {
        if (!std::isfinite(data.values[i])) continue;
        dataMin = std::min(dataMin, data.values[i]);
        dataMax = std::max(dataMax, data.values[i]);
    }
    if (!std::isfinite(dataMin) || !std::isfinite(dataMax)) {
        dataMin = 0.0;
        dataMax = 1.0;
    }
    const AxisScale yScale = niceScale(dataMin, dataMax, 6, options.includeZero);

    // --- legend entries (one per category, palette colour by position) ---
    std::vector<LegendEntry> legendEntries;
    if (options.showLegend && n > 0) {
        legendEntries.reserve(n);
        for (std::size_t i = 0; i < n; ++i)
            legendEntries.push_back(LegendEntry{data.categories[i], paletteColor(i)});
    }

    // --- widest y tick label, so ChartLayout leaves enough room on the left ---
    FontManager& font = FontManager::defaultFont();
    double widestYLabel = 0.0;
    for (const std::string& label : tickLabels(yScale))
        widestYLabel = std::max(widestYLabel, measureText(font, label, options.tickLabelSize).width);

    const double legendWidth = options.showLegend && !legendEntries.empty()
                                    ? measureLegendWidth(legendEntries, options.legendLabelSize)
                                    : 0.0;

    LayoutOptions layoutOpts;
    layoutOpts.hasTitle = !options.title.empty();
    layoutOpts.hasXAxisTitle = !options.xAxisTitle.empty();
    layoutOpts.hasYAxisTitle = !options.yAxisTitle.empty();
    layoutOpts.hasLegend = options.showLegend && !legendEntries.empty();
    layoutOpts.titleSize = options.titleSize;
    layoutOpts.axisTitleSize = options.axisTitleSize;
    layoutOpts.tickLabelSize = options.tickLabelSize;
    layoutOpts.yTickLabelWidth = widestYLabel;
    layoutOpts.legendWidth = legendWidth;

    const ChartLayout layout = computeChartLayout(width, height, layoutOpts);
    const PixelRect& plotArea = layout.plotArea;

    // x domain is purely categorical: [0, n]. Category i occupies [i, i+1).
    // n == 0 is safe: Viewport repairs a zero-width range on its own.
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

    // --- grid, drawn before the bars so bars sit on top ---
    drawHorizontalGrid(r, vp, yScale.ticks, options.gridColor);
    drawZeroLines(r, vp, options.zeroLineColor);

    // --- bars, clipped to the plot area so extreme values never bleed out ---
    const double slotWidth = n > 0 ? plotArea.width / static_cast<double>(n) : 0.0;
    const double barWidth = std::max(1.0, slotWidth * (1.0 - options.barGapRatio));
    const double baseline = chooseBaseline(yScale);
    const double baselinePixelY = vp.mapY(baseline);

    std::vector<double> categoryCentersX(n);
    for (std::size_t i = 0; i < n; ++i)
        categoryCentersX[i] = vp.mapX(static_cast<double>(i) + 0.5);

    if (plotArea.width > 0.0 && plotArea.height > 0.0) {
        r.context().setClip(roundToInt(plotArea.left), roundToInt(plotArea.top),
                            roundToInt(plotArea.right()), roundToInt(plotArea.bottom()));
        for (std::size_t i = 0; i < n; ++i) {
            if (!std::isfinite(data.values[i])) continue;
            const double valuePixelY = vp.mapY(data.values[i]);
            if (!std::isfinite(valuePixelY)) continue;

            const double top = std::min(valuePixelY, baselinePixelY);
            const double barHeight = std::fabs(baselinePixelY - valuePixelY);

            Rectangle bar;
            bar.x = categoryCentersX[i] - barWidth / 2.0;
            bar.y = top;
            bar.width = barWidth;
            bar.height = barHeight;
            bar.fill = paletteColor(i);
            r.draw(bar);
        }
        r.context().clearClip();
    }

    // --- frame, ticks, category labels ---
    const AxisStyle axisStyle{options.axisColor, Color(40, 40, 40), options.tickLabelSize};
    drawPlotFrame(r, plotArea, axisStyle);
    drawYAxisTicks(r, vp, yScale, layout, axisStyle);

    const AxisStyle categoryStyle{options.axisColor, Color(40, 40, 40), options.categoryLabelSize};
    drawCategoryLabels(r, categoryCentersX, data.categories, slotWidth, plotArea, layout, categoryStyle);

    // --- legend ---
    if (layoutOpts.hasLegend)
        drawLegend(r, layout.legendArea, legendEntries, options.legendLabelSize, Color(40, 40, 40));

    // --- titles ---
    drawTitle(r, layout, options.title, options.titleSize, options.titleColor);
    drawXAxisTitle(r, layout, options.xAxisTitle, options.axisTitleSize, options.axisTitleColor);
    drawYAxisTitle(r, layout, options.yAxisTitle, options.axisTitleSize, options.axisTitleColor);
}

}  // namespace gfx
