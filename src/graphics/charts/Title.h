#pragma once
#include <string>

#include "graphics/Color.h"
#include "graphics/Renderer.h"
#include "graphics/layout/ChartLayout.h"

namespace gfx {

// Chart title: centred over the plot area, at the top of the canvas.
// Too-long titles are shortened with "..." and the text is kept inside the canvas.
// Empty text draws nothing.
void drawTitle(Renderer& r, const ChartLayout& layout, const std::string& text, double size,
               const Color& color);

// X-axis title: centred under the plot area, at the bottom of the canvas.
void drawXAxisTitle(Renderer& r, const ChartLayout& layout, const std::string& text, double size,
                    const Color& color);

// Y-axis title: rotated to read bottom-to-top, at the left edge of the canvas.
// Shortened to fit the plot height.
void drawYAxisTitle(Renderer& r, const ChartLayout& layout, const std::string& text, double size,
                    const Color& color);

}  // namespace gfx
