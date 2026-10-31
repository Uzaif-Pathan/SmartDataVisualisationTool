#pragma once
#include <string>
#include <vector>

#include "graphics/Color.h"
#include "graphics/GraphicsContext.h"
#include "graphics/text/FontManager.h"

namespace gfx {

// Which part of the text sits at the anchor point (x, y).
enum class HAlign { Left, Center, Right };
enum class VAlign { Baseline, Top, Middle, Bottom };

// None = normal, left to right.
// Up   = rotated 90 degrees anticlockwise (reads bottom-to-top), for y-axis titles.
enum class TextRotation { None, Up };

struct TextMetrics {
    double width = 0.0;     // how far the pen advances over the whole string
    double height = 0.0;    // ascent + descent
    double ascent = 0.0;
    double descent = 0.0;
};

// Converts UTF-8 text to Unicode code points.
// Invalid bytes become '?'; control characters (newline, tab, ...) become spaces.
std::vector<int> decodeUtf8(const std::string& text);

// Size of the text.  Returns all zeros if no font is loaded or size is invalid.
TextMetrics measureText(FontManager& font, const std::string& text, double pixelSize);

// Draws one line of anti-aliased text with (x, y) as the anchor point.
// Draws nothing (no error) for empty text, no font, or NaN/infinite/non-positive input.
void drawText(GraphicsContext& ctx, FontManager& font, const std::string& text,
              double x, double y, double pixelSize, const Color& color,
              HAlign hAlign, VAlign vAlign, TextRotation rotation);

}  // namespace gfx
