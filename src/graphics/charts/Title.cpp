#include "graphics/charts/Title.h"

#include <algorithm>
#include <cmath>

#include "graphics/charts/ChartText.h"
#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

namespace gfx {

void drawTitle(Renderer& r, const ChartLayout& layout, const std::string& text, double size,
               const Color& color) {
    if (text.empty()) return;
    const double canvasW = r.canvas().width();
    const double maxWidth = canvasW - 2.0 * layout.padding;
    const std::string fitted = fitText(text, size, maxWidth);
    if (fitted.empty()) return;

    // Centre on the plot area, but slide sideways if that would push the text off the canvas.
    const double w = measureText(FontManager::defaultFont(), fitted, size).width;
    const double lo = layout.padding + w / 2.0;
    const double hi = canvasW - layout.padding - w / 2.0;
    const double x = (lo <= hi) ? std::max(lo, std::min(hi, layout.titleX)) : canvasW / 2.0;
    r.draw(makeLabel(fitted, x, layout.titleY, size, color, HAlign::Center, VAlign::Top));
}

void drawXAxisTitle(Renderer& r, const ChartLayout& layout, const std::string& text, double size,
                    const Color& color) {
    if (text.empty()) return;
    const std::string fitted = fitText(text, size, layout.plotArea.width);
    if (fitted.empty()) return;
    r.draw(makeLabel(fitted, layout.xTitleX, layout.xTitleY, size, color, HAlign::Center,
                     VAlign::Bottom));
}

void drawYAxisTitle(Renderer& r, const ChartLayout& layout, const std::string& text, double size,
                    const Color& color) {
    if (text.empty()) return;
    const std::string fitted = fitText(text, size, layout.plotArea.height);
    if (fitted.empty()) return;
    r.draw(makeLabel(fitted, layout.yTitleX, layout.yTitleY, size, color, HAlign::Center,
                     VAlign::Middle, TextRotation::Up));
}

}  // namespace gfx
