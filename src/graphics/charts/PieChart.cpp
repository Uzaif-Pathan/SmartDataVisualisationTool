#include "graphics/charts/PieChart.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

#include "graphics/charts/ChartText.h"
#include "graphics/charts/Legend.h"
#include "graphics/charts/Title.h"
#include "graphics/layout/ChartLayout.h"
#include "graphics/primitives/Circle.h"
#include "graphics/primitives/Polygon.h"
#include "graphics/primitives/Rectangle.h"
#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

namespace gfx {
namespace {

constexpr double kPi = 3.14159265358979323846;

double sanitizeSize(double v, double fallback) {
    return (std::isfinite(v) && v > 0.0) ? v : fallback;
}

// A point at the given angle (degrees, 0 = 12 o'clock) around (cx, cy).
Vertex pointOnCircle(double cx, double cy, double radius, double angleDeg, bool clockwise) {
    const double rad = angleDeg * kPi / 180.0;
    const double signedRad = clockwise ? rad : -rad;
    return Vertex{cx + radius * std::sin(signedRad), cy - radius * std::cos(signedRad)};
}

// "34%", or "<1%" for a non-zero share that would otherwise round to 0.
std::string formatPercent(double fraction) {
    const double percent = fraction * 100.0;
    if (percent > 0.0 && percent < 0.5) return "<1%";
    std::ostringstream oss;
    oss << std::lround(percent) << "%";
    return oss.str();
}

// White text on a dark slice, dark text on a light one.
Color labelColorFor(const Color& sliceColor) {
    const double luminance =
        0.299 * sliceColor.r + 0.587 * sliceColor.g + 0.114 * sliceColor.b;
    return luminance > 140.0 ? Color(30, 30, 30) : colors::White;
}

struct ValidSlice {
    std::string label;
    double value;
};

}  // namespace

void drawPieChart(Renderer& r, double width, double height, const PieChartData& data,
                  const PieChartOptions& optionsIn) {
    PieChartOptions options = optionsIn;
    options.titleSize = sanitizeSize(options.titleSize, 20.0);
    options.labelSize = sanitizeSize(options.labelSize, 13.0);
    options.legendLabelSize = sanitizeSize(options.legendLabelSize, 12.0);
    options.minLabelPercent = std::isfinite(options.minLabelPercent)
                                  ? std::max(0.0, std::min(1.0, options.minLabelPercent))
                                  : 0.03;
    options.startAngleDeg = std::isfinite(options.startAngleDeg) ? options.startAngleDeg : 0.0;
    options.sliceStrokeWidth = std::max(0, options.sliceStrokeWidth);

    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0 || height <= 0.0) return;

    // A pie chart has no natural zero-width slot for a bad value (unlike a
    // bar chart's category axis), so invalid or non-positive slices are
    // dropped entirely rather than kept as an empty wedge.
    std::vector<ValidSlice> valid;
    valid.reserve(data.slices.size());
    for (const PieSlice& s : data.slices)
        if (std::isfinite(s.value) && s.value > 0.0) valid.push_back({s.label, s.value});

    double total = 0.0;
    for (const ValidSlice& s : valid) total += s.value;

    // --- legend entries (one per valid slice, palette colour by position) ---
    std::vector<LegendEntry> legendEntries;
    if (options.showLegend && !valid.empty() && total > 0.0) {
        legendEntries.reserve(valid.size());
        for (std::size_t i = 0; i < valid.size(); ++i) {
            const std::string text = valid[i].label + " (" + formatPercent(valid[i].value / total) + ")";
            legendEntries.push_back(LegendEntry{text, paletteColor(i)});
        }
    }
    const double legendWidth = !legendEntries.empty()
                                  ? measureLegendWidth(legendEntries, options.legendLabelSize)
                                  : 0.0;

    // No axes at all, so tick/axis-title fields stay at 0 to avoid reserving
    // margin space this chart never uses.
    LayoutOptions layoutOpts;
    layoutOpts.hasTitle = !options.title.empty();
    layoutOpts.hasLegend = !legendEntries.empty();
    layoutOpts.titleSize = options.titleSize;
    layoutOpts.legendWidth = legendWidth;

    const ChartLayout layout = computeChartLayout(width, height, layoutOpts);
    const PixelRect& plotArea = layout.plotArea;

    if (plotArea.width > 0.0 && plotArea.height > 0.0) {
        Rectangle bg;
        bg.x = plotArea.left;
        bg.y = plotArea.top;
        bg.width = plotArea.width;
        bg.height = plotArea.height;
        bg.fill = options.plotBackground;
        r.draw(bg);
    }

    const double cx = plotArea.centerX();
    const double cy = plotArea.centerY();
    const double margin = 4.0;
    const double radius = std::max(0.0, std::min(plotArea.width, plotArea.height) / 2.0 - margin);

    if (radius > 0.0) {
        if (valid.empty() || total <= 0.0) {
            // Nothing to show: an empty ring rather than a blank hole, so the
            // chart still reads as "a pie chart with no data" and not a bug.
            strokeCircle(r.context(), cx, cy, radius, 1.5, Color(200, 200, 200));
        } else if (valid.size() == 1) {
            // A single slice is the whole circle; going through the wedge
            // math below would need a degenerate 360-degree arc.
            const Color color = paletteColor(0);
            fillCircle(r.context(), cx, cy, radius, color);
            if (options.sliceStrokeWidth > 0) strokeCircle(r.context(), cx, cy, radius, 1.5, options.sliceStrokeColor);
            if (options.showPercentLabels) {
                Text label = makeLabel("100%", cx, cy, options.labelSize, labelColorFor(color),
                                       HAlign::Center, VAlign::Middle);
                r.draw(label);
            }
        } else {
            double angle = options.startAngleDeg;
            for (std::size_t i = 0; i < valid.size(); ++i) {
                const double fraction = valid[i].value / total;
                const double sweepDeg = fraction * 360.0;
                const double angleStart = angle;
                const double angleEnd = angle + sweepDeg;
                angle = angleEnd;

                const Color color = paletteColor(i);

                if (sweepDeg > 0.0) {
                    const int segments = std::max(1, static_cast<int>(std::ceil(sweepDeg / 4.0)));
                    Polygon wedge;
                    wedge.vertices.reserve(static_cast<std::size_t>(segments) + 2);
                    wedge.vertices.push_back(Vertex{cx, cy});
                    for (int k = 0; k <= segments; ++k) {
                        const double t = static_cast<double>(k) / static_cast<double>(segments);
                        const double a = angleStart + sweepDeg * t;
                        wedge.vertices.push_back(pointOnCircle(cx, cy, radius, a, options.clockwise));
                    }
                    wedge.fill = color;
                    if (options.sliceStrokeWidth > 0) {
                        wedge.stroke = options.sliceStrokeColor;
                        wedge.strokeWidth = options.sliceStrokeWidth;
                    }
                    r.draw(wedge);
                }

                if (options.showPercentLabels && fraction >= options.minLabelPercent) {
                    const double midAngle = angleStart + sweepDeg / 2.0;
                    const Vertex labelPos = pointOnCircle(cx, cy, radius * 0.65, midAngle, options.clockwise);
                    Text label = makeLabel(formatPercent(fraction), labelPos.x, labelPos.y,
                                           options.labelSize, labelColorFor(color), HAlign::Center,
                                           VAlign::Middle);
                    r.draw(label);
                }
            }
        }
    }

    if (layoutOpts.hasLegend)
        drawLegend(r, layout.legendArea, legendEntries, options.legendLabelSize, Color(40, 40, 40));

    drawTitle(r, layout, options.title, options.titleSize, options.titleColor);
}

}  // namespace gfx
