#include "graphics/charts/Histogram.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "graphics/Viewport.h"
#include "graphics/charts/Axis.h"
#include "graphics/charts/Grid.h"
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

double clamp0to05(double v) {
    if (!std::isfinite(v)) return 0.02;
    return std::max(0.0, std::min(0.5, v));
}

// Sturges' rule: k = ceil(log2(n) + 1), minimum 1 bin.
int sturgesBinCount(std::size_t n) {
    if (n <= 1) return 1;
    const int k = static_cast<int>(std::ceil(std::log2(static_cast<double>(n)) + 1.0));
    return std::max(1, k);
}

}  // namespace

void drawHistogram(Renderer& r, double width, double height, const HistogramData& data,
                   const HistogramOptions& optionsIn) {
    HistogramOptions options = optionsIn;
    options.titleSize = sanitizeSize(options.titleSize, 20.0);
    options.axisTitleSize = sanitizeSize(options.axisTitleSize, 14.0);
    options.tickLabelSize = sanitizeSize(options.tickLabelSize, 12.0);
    options.barGapRatio = clamp0to05(options.barGapRatio);

    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0 || height <= 0.0) return;

    // --- gather finite values only ---
    std::vector<double> values;
    values.reserve(data.values.size());
    for (double v : data.values)
        if (std::isfinite(v)) values.push_back(v);

    double dataMin = 0.0, dataMax = 1.0;
    if (!values.empty()) {
        dataMin = *std::min_element(values.begin(), values.end());
        dataMax = *std::max_element(values.begin(), values.end());
    }
    if (dataMin == dataMax) {
        // Constant (or single) value: widen so the one bin has visible width.
        const double delta = (dataMin == 0.0) ? 1.0 : std::fabs(dataMin) * 0.1;
        dataMin -= delta;
        dataMax += delta;
    }

    const int binCount = options.binCount > 0 ? options.binCount : sturgesBinCount(values.size());
    const double binWidth = (dataMax - dataMin) / static_cast<double>(binCount);

    std::vector<double> counts(static_cast<std::size_t>(binCount), 0.0);
    for (double v : values) {
        int bin = binWidth > 0.0 ? static_cast<int>((v - dataMin) / binWidth) : 0;
        if (bin < 0) bin = 0;
        if (bin >= binCount) bin = binCount - 1;   // the max value lands in the last bin
        counts[static_cast<std::size_t>(bin)] += 1.0;
    }

    double maxCount = 0.0;
    for (double c : counts) maxCount = std::max(maxCount, c);

    // --- axes ---
    const AxisScale xScale = niceScale(dataMin, dataMax, 6, false);
    const AxisScale yScale = niceScale(0.0, maxCount, 6, true);

    // --- widest y tick label, so ChartLayout leaves enough room on the left ---
    FontManager& font = FontManager::defaultFont();
    double widestYLabel = 0.0;
    for (const std::string& label : tickLabels(yScale))
        widestYLabel = std::max(widestYLabel, measureText(font, label, options.tickLabelSize).width);

    LayoutOptions layoutOpts;
    layoutOpts.hasTitle = !options.title.empty();
    layoutOpts.hasXAxisTitle = !options.xAxisTitle.empty();
    layoutOpts.hasYAxisTitle = !options.yAxisTitle.empty();
    layoutOpts.hasLegend = false;
    layoutOpts.titleSize = options.titleSize;
    layoutOpts.axisTitleSize = options.axisTitleSize;
    layoutOpts.tickLabelSize = options.tickLabelSize;
    layoutOpts.yTickLabelWidth = widestYLabel;

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

    // --- grid, drawn before the bars so bars sit on top ---
    drawHorizontalGrid(r, vp, yScale.ticks, options.gridColor);
    drawZeroLines(r, vp, options.zeroLineColor);

    // --- bars, one per bin, contiguous across the data range ---
    if (plotArea.width > 0.0 && plotArea.height > 0.0) {
        r.context().setClip(roundToInt(plotArea.left), roundToInt(plotArea.top),
                            roundToInt(plotArea.right()), roundToInt(plotArea.bottom()));
        const double baselinePixelY = vp.mapY(0.0);
        for (int i = 0; i < binCount; ++i) {
            const double edgeLo = dataMin + i * binWidth;
            const double edgeHi = dataMin + (i + 1) * binWidth;
            const double gap = (edgeHi - edgeLo) * options.barGapRatio / 2.0;

            const double pxLo = vp.mapX(edgeLo + gap);
            const double pxHi = vp.mapX(edgeHi - gap);
            const double valuePixelY = vp.mapY(counts[static_cast<std::size_t>(i)]);
            if (!std::isfinite(pxLo) || !std::isfinite(pxHi) || !std::isfinite(valuePixelY)) continue;

            Rectangle bar;
            bar.x = std::min(pxLo, pxHi);
            bar.y = std::min(valuePixelY, baselinePixelY);
            bar.width = std::max(1.0, std::fabs(pxHi - pxLo));
            bar.height = std::fabs(baselinePixelY - valuePixelY);
            bar.fill = options.barColor;
            r.draw(bar);
        }
        r.context().clearClip();
    }

    // --- frame and ticks ---
    const AxisStyle axisStyle{options.axisColor, Color(40, 40, 40), options.tickLabelSize};
    drawPlotFrame(r, plotArea, axisStyle);
    drawXAxisTicks(r, vp, xScale, layout, axisStyle);
    drawYAxisTicks(r, vp, yScale, layout, axisStyle);

    // --- titles ---
    drawTitle(r, layout, options.title, options.titleSize, options.titleColor);
    drawXAxisTitle(r, layout, options.xAxisTitle, options.axisTitleSize, options.axisTitleColor);
    drawYAxisTitle(r, layout, options.yAxisTitle, options.axisTitleSize, options.axisTitleColor);
}

}  // namespace gfx
