#pragma once
#include <string>

#include "graphics/Color.h"
#include "graphics/GraphicsContext.h"
#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

namespace gfx {

// One line of text.  (x, y) is the anchor point; hAlign/vAlign say which part
// of the text sits on it.  For example, an x-axis tick label uses
// Center + Top, and a y-axis tick label uses Right + Middle.
// size is the font height in pixels.  If `font` is null, the shared default
// font (DejaVu Sans) is used.
struct Text {
    std::string content;
    double x = 0.0;
    double y = 0.0;
    Color color = colors::Black;
    double size = 14.0;
    HAlign hAlign = HAlign::Left;
    VAlign vAlign = VAlign::Baseline;
    TextRotation rotation = TextRotation::None;
    FontManager* font = nullptr;

    void draw(GraphicsContext& ctx) const;
    TextMetrics measure() const;
};

}  // namespace gfx
