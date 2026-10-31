#include "graphics/primitives/Text.h"

namespace gfx {

void Text::draw(GraphicsContext& ctx) const {
    FontManager& f = font ? *font : FontManager::defaultFont();
    drawText(ctx, f, content, x, y, size, color, hAlign, vAlign, rotation);
}

TextMetrics Text::measure() const {
    FontManager& f = font ? *font : FontManager::defaultFont();
    return measureText(f, content, size);
}

}  // namespace gfx
