#include "graphics/layout/ChartLayout.h"

#include <algorithm>
#include <cmath>

namespace gfx {
namespace {

constexpr double kLineFactor = 1.25;   // line height = font size * 1.25
constexpr double kMinPlot = 10.0;

double nonNegative(double v) { return (std::isfinite(v) && v > 0.0) ? v : 0.0; }

// Shrinks two opposite margins if they would leave less than kMinPlot pixels.
void fitMargins(double total, double& a, double& b) {
    if (total - a - b >= kMinPlot) return;
    const double available = std::max(0.0, total - std::min(total, kMinPlot));
    const double sum = a + b;
    const double scale = sum > 0.0 ? std::min(1.0, available / sum) : 1.0;
    a *= scale;
    b *= scale;
}

}  // namespace

ChartLayout computeChartLayout(double width, double height, const LayoutOptions& o) {
    ChartLayout L;
    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0 || height <= 0.0)
        return L;

    const double titleSize = nonNegative(o.titleSize);
    const double axisSize = nonNegative(o.axisTitleSize);
    const double tickSize = nonNegative(o.tickLabelSize);
    const double yLabelWidth = nonNegative(o.yTickLabelWidth);
    const double legendWidth = nonNegative(o.legendWidth);

    const double minDim = std::min(width, height);
    const double pad = std::max(6.0, std::min(20.0, minDim * 0.03));
    const double tickLen = std::max(3.0, std::min(6.0, minDim * 0.01));
    const double gap = 3.0;

    // Margins, measured from the canvas edges inward.
    double top = pad;
    if (o.hasTitle) top += titleSize * kLineFactor + pad * 0.75;

    double bottom = pad + tickLen + gap + tickSize * kLineFactor;
    if (o.hasXAxisTitle) bottom += gap + axisSize * kLineFactor;

    double left = pad + tickLen + gap + yLabelWidth;
    if (o.hasYAxisTitle) left += gap + axisSize * kLineFactor;

    // Extra room on the right so the last x label (centred on the right edge
    // of the plot) is not cut off.
    double right = pad + tickSize;
    if (o.hasLegend && legendWidth > 0.0) right += pad + legendWidth;

    fitMargins(width, left, right);
    fitMargins(height, top, bottom);

    L.padding = pad;
    L.tickLength = tickLen;
    L.tickLabelGap = gap;
    L.plotArea = PixelRect{left, top, width - left - right, height - top - bottom};

    L.titleX = L.plotArea.centerX();
    L.titleY = pad;
    L.xTitleX = L.plotArea.centerX();
    L.xTitleY = height - pad;
    L.yTitleX = pad + axisSize * kLineFactor / 2.0;
    L.yTitleY = L.plotArea.centerY();

    if (o.hasLegend && legendWidth > 0.0) {
        const double lx = std::max(width - pad - legendWidth, L.plotArea.right());
        L.legendArea = PixelRect{lx, L.plotArea.top, std::max(0.0, width - pad - lx),
                                 L.plotArea.height};
    }
    return L;
}

}  // namespace gfx
