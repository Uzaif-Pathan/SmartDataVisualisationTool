#pragma once
#include <optional>

#include "graphics/Color.h"
#include "graphics/GraphicsContext.h"

namespace gfx {

// A rectangle with its top-left corner at (x, y).
// Covers pixels round(x) .. round(x+width)-1 (the right/bottom edge is
// exclusive), so neighbouring bars never overlap or leave gaps.
// A rectangle that is thin but not empty is drawn at least 1 pixel wide/tall,
// so a tiny bar never disappears.  width <= 0 or height <= 0 draws nothing:
// callers must normalise negative sizes themselves (bar charts do this).
// The outline is drawn INSIDE the rectangle's bounds.
struct Rectangle {
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
    std::optional<Color> fill;
    std::optional<Color> stroke;
    int strokeWidth = 1;

    void draw(GraphicsContext& ctx) const;
};

}  // namespace gfx
