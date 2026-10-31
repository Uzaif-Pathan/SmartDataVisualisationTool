#include "graphics/charts/Axis.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "graphics/charts/ChartText.h"
#include "graphics/primitives/Line.h"
#include "graphics/primitives/Rectangle.h"
#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

namespace gfx {
namespace {

// How many ticks to jump so labels of size `labelExtent` (pixels along the axis)
// fit into `spacing` pixels.
std::size_t strideFor(double labelExtent, double spacing) {
    if (!(spacing > 0.0) || !std::isfinite(spacing) || !(labelExtent > 0.0)) return 1;
    const double s = std::ceil(labelExtent / spacing);
    if (!(s >= 1.0)) return 1;
    if (s > 1000.0) return 1000;
    return static_cast<std::size_t>(s);
}

}  // namespace

void drawPlotFrame(Renderer& r, const PixelRect& a, const AxisStyle& style) {
    if (!(a.width > 0.0) || !(a.height > 0.0)) return;
    Rectangle border;
    border.x = a.left;
    border.y = a.top;
    border.width = a.width;
    border.height = a.height;
    border.stroke = style.lineColor;
    r.draw(border);
}

void drawXAxisTicks(Renderer& r, const Viewport& vp, const AxisScale& scale,
                    const ChartLayout& layout, const AxisStyle& style) {
    const PixelRect& a = vp.area();
    if (!(a.width > 0.0) || scale.ticks.empty()) return;

    FontManager& font = FontManager::defaultFont();
    const std::vector<std::string> labels = tickLabels(scale);

    double widest = 0.0;
    for (const std::string& s : labels)
        widest = std::max(widest, measureText(font, s, style.labelSize).width);

    double spacing = a.width;
    if (scale.ticks.size() > 1)
        spacing = std::fabs(vp.mapX(scale.ticks[0] + scale.step) - vp.mapX(scale.ticks[0]));
    const std::size_t stride = strideFor(widest + 6.0, spacing);

    for (std::size_t i = 0; i < scale.ticks.size(); ++i) {
        const double px = vp.mapX(scale.ticks[i]);
        if (!std::isfinite(px)) continue;
        r.draw(Line{px, a.bottom(), px, a.bottom() + layout.tickLength, style.lineColor, 1});
        if (i % stride != 0) continue;
        r.draw(makeLabel(labels[i], px, a.bottom() + layout.tickLength + layout.tickLabelGap,
                         style.labelSize, style.labelColor, HAlign::Center, VAlign::Top));
    }
}

void drawYAxisTicks(Renderer& r, const Viewport& vp, const AxisScale& scale,
                    const ChartLayout& layout, const AxisStyle& style) {
    const PixelRect& a = vp.area();
    if (!(a.height > 0.0) || scale.ticks.empty()) return;

    const std::vector<std::string> labels = tickLabels(scale);

    double spacing = a.height;
    if (scale.ticks.size() > 1)
        spacing = std::fabs(vp.mapY(scale.ticks[0] + scale.step) - vp.mapY(scale.ticks[0]));
    const std::size_t stride = strideFor(style.labelSize * 1.25, spacing);

    for (std::size_t i = 0; i < scale.ticks.size(); ++i) {
        const double py = vp.mapY(scale.ticks[i]);
        if (!std::isfinite(py)) continue;
        r.draw(Line{a.left - layout.tickLength, py, a.left, py, style.lineColor, 1});
        if (i % stride != 0) continue;
        r.draw(makeLabel(labels[i], a.left - layout.tickLength - layout.tickLabelGap, py,
                         style.labelSize, style.labelColor, HAlign::Right, VAlign::Middle));
    }
}

void drawCategoryLabels(Renderer& r, const std::vector<double>& centersX,
                        const std::vector<std::string>& labels, double slotWidth,
                        const PixelRect& plotArea, const ChartLayout& layout,
                        const AxisStyle& style) {
    if (!std::isfinite(slotWidth) || slotWidth <= 0.0) return;
    if (!(plotArea.width > 0.0) || !(plotArea.height > 0.0)) return;

    const std::size_t n = std::min(centersX.size(), labels.size());
    const std::size_t stride = strideFor(18.0, slotWidth);   // never fewer than ~18 px per label
    const double maxWidth = slotWidth * static_cast<double>(stride) - 4.0;
    if (maxWidth < 8.0) return;

    for (std::size_t i = 0; i < n; i += stride) {
        const double px = centersX[i];
        if (!std::isfinite(px)) continue;
        r.draw(Line{px, plotArea.bottom(), px, plotArea.bottom() + layout.tickLength,
                    style.lineColor, 1});
        const std::string text = fitText(labels[i], style.labelSize, maxWidth);
        if (text.empty()) continue;
        r.draw(makeLabel(text, px, plotArea.bottom() + layout.tickLength + layout.tickLabelGap,
                         style.labelSize, style.labelColor, HAlign::Center, VAlign::Top));
    }
}

}  // namespace gfx
