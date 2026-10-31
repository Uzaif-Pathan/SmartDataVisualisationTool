#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "graphics/Color.h"
#include "graphics/Renderer.h"
#include "graphics/Viewport.h"

namespace gfx {

struct LegendEntry {
    std::string label;
    Color color;
};

// A fixed set of 10 easy-to-tell-apart colours.  index wraps around, so any
// number of series or categories gets a colour.
const Color& paletteColor(std::size_t index);

// Width in pixels the legend needs: swatch + gap + the widest label (labels
// wider than 160 px are counted as 160).  0 for an empty list or bad size.
double measureLegendWidth(const std::vector<LegendEntry>& entries, double fontSize);

// Draws one row per entry (colour square + label) inside `area`, from the top.
// Labels are shortened to fit the width.  If not every row fits vertically, the
// last row becomes "+N more".  Draws nothing for empty entries, an empty area,
// or a bad font size.
void drawLegend(Renderer& r, const PixelRect& area, const std::vector<LegendEntry>& entries,
                double fontSize, const Color& textColor);

}  // namespace gfx
