#include "graphics/charts/Legend.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "graphics/charts/ChartText.h"
#include "graphics/primitives/Rectangle.h"
#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

namespace gfx {
namespace {

constexpr double kGap = 6.0;              // space between swatch and label
constexpr double kMaxLabelWidth = 160.0;

double swatchSize(double fontSize) { return fontSize * 0.9; }
double rowHeight(double fontSize) { return fontSize * 1.6; }

}  // namespace

const Color& paletteColor(std::size_t index) {
    static const Color kPalette[] = {
        Color(31, 119, 180), Color(255, 127, 14), Color(44, 160, 44), Color(214, 39, 40),
        Color(148, 103, 189), Color(140, 86, 75), Color(227, 119, 194), Color(127, 127, 127),
        Color(188, 189, 34), Color(23, 190, 207)};
    return kPalette[index % (sizeof(kPalette) / sizeof(kPalette[0]))];
}

double measureLegendWidth(const std::vector<LegendEntry>& entries, double fontSize) {
    if (entries.empty() || !std::isfinite(fontSize) || fontSize <= 0.0) return 0.0;
    FontManager& font = FontManager::defaultFont();
    double widest = 0.0;
    for (const LegendEntry& e : entries)
        widest = std::max(widest, measureText(font, e.label, fontSize).width);
    widest = std::min(widest, kMaxLabelWidth);
    return swatchSize(fontSize) + kGap + widest + 4.0;
}

void drawLegend(Renderer& r, const PixelRect& area, const std::vector<LegendEntry>& entries,
                double fontSize, const Color& textColor) {
    if (entries.empty() || !(area.width > 0.0) || !(area.height > 0.0)) return;
    if (!std::isfinite(fontSize) || fontSize <= 0.0) return;

    const double rowH = rowHeight(fontSize);
    const double sw = swatchSize(fontSize);
    const std::size_t capacity = static_cast<std::size_t>(std::floor(area.height / rowH));
    if (capacity == 0) return;

    std::size_t shown = entries.size();
    bool more = false;
    if (entries.size() > capacity) {
        more = true;
        shown = capacity >= 2 ? capacity - 1 : 0;
    }

    const double textX = area.left + sw + kGap;
    const double textWidth = area.width - sw - kGap - 2.0;

    for (std::size_t i = 0; i < shown; ++i) {
        const double cy = area.top + rowH * static_cast<double>(i) + rowH / 2.0;
        Rectangle box;
        box.x = area.left;
        box.y = cy - sw / 2.0;
        box.width = sw;
        box.height = sw;
        box.fill = entries[i].color;
        r.draw(box);
        const std::string label = fitText(entries[i].label, fontSize, textWidth);
        if (label.empty()) continue;
        r.draw(makeLabel(label, textX, cy, fontSize, textColor, HAlign::Left, VAlign::Middle));
    }

    if (more) {
        const double cy = area.top + rowH * static_cast<double>(shown) + rowH / 2.0;
        const std::string text = "+" + std::to_string(entries.size() - shown) + " more";
        const std::string fitted = fitText(text, fontSize, area.width);
        if (!fitted.empty())
            r.draw(makeLabel(fitted, area.left, cy, fontSize, textColor, HAlign::Left,
                             VAlign::Middle));
    }
}

}  // namespace gfx
