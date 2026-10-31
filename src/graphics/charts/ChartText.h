#pragma once
#include <string>

#include "graphics/Color.h"
#include "graphics/primitives/Text.h"

namespace gfx {

// Short way to build a Text with the fields charts always set.
inline Text makeLabel(const std::string& content, double x, double y, double size,
                      const Color& color, HAlign h, VAlign v,
                      TextRotation rotation = TextRotation::None) {
    Text t;
    t.content = content;
    t.x = x;
    t.y = y;
    t.size = size;
    t.color = color;
    t.hAlign = h;
    t.vAlign = v;
    t.rotation = rotation;
    return t;
}

// Shortens `text` so it is at most maxWidth pixels wide at the given font size.
//  - If it already fits, it is returned unchanged.
//  - Otherwise characters are removed from the end and a "..." character is
//    added.  Cuts happen on UTF-8 character boundaries, never in the middle.
//  - Returns "" if maxWidth is NaN/infinite/<= 0, or even the "..." does not fit.
// Uses the shared default font.
std::string fitText(const std::string& text, double pixelSize, double maxWidth);

}  // namespace gfx
