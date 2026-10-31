#include "graphics/text/TextRenderer.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gfx {

std::vector<int> decodeUtf8(const std::string& s) {
    std::vector<int> out;
    std::size_t i = 0;
    const std::size_t n = s.size();
    while (i < n) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        int cp = 0;
        std::size_t len = 0;
        if (c < 0x80)              { cp = c;        len = 1; }
        else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; len = 2; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; len = 3; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; len = 4; }
        else { out.push_back('?'); ++i; continue; }          // invalid first byte

        if (i + len > n) { out.push_back('?'); break; }      // text ends mid-character

        bool ok = true;
        for (std::size_t k = 1; k < len; ++k) {
            const unsigned char cc = static_cast<unsigned char>(s[i + k]);
            if ((cc & 0xC0) != 0x80) { ok = false; break; }
            cp = (cp << 6) | (cc & 0x3F);
        }
        if (!ok || cp > 0x10FFFF) { out.push_back('?'); ++i; continue; }

        if (cp < 0x20 || (cp >= 0x7F && cp < 0xA0)) cp = ' ';   // control characters
        out.push_back(cp);
        i += len;
    }
    return out;
}

TextMetrics measureText(FontManager& font, const std::string& text, double pixelSize) {
    TextMetrics m;
    if (!font.isLoaded() || !std::isfinite(pixelSize) || pixelSize <= 0.0) return m;

    const FontMetrics fm = font.metrics(pixelSize);
    m.ascent = fm.ascent;
    m.descent = fm.descent;
    m.height = fm.ascent + fm.descent;

    const std::vector<int> cps = decodeUtf8(text);
    double pen = 0.0;
    for (std::size_t i = 0; i < cps.size(); ++i) {
        pen += font.glyph(cps[i], pixelSize).advance;
        if (i + 1 < cps.size()) pen += font.kerning(cps[i], cps[i + 1], pixelSize);
    }
    m.width = pen;
    return m;
}

void drawText(GraphicsContext& ctx, FontManager& font, const std::string& text,
              double x, double y, double pixelSize, const Color& color,
              HAlign hAlign, VAlign vAlign, TextRotation rotation) {
    if (!font.isLoaded() || text.empty()) return;
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(pixelSize) || pixelSize <= 0.0)
        return;

    const TextMetrics m = measureText(font, text, pixelSize);

    // Work in the text's own frame: the anchor is (0,0), x runs along the
    // text, y runs "down" through the letters.  Rotation is applied last.
    double startX = 0.0;
    if (hAlign == HAlign::Center) startX = -m.width / 2.0;
    else if (hAlign == HAlign::Right) startX = -m.width;

    double baseline = 0.0;    // where the baseline sits relative to the anchor
    switch (vAlign) {
        case VAlign::Baseline: baseline = 0.0; break;
        case VAlign::Top:      baseline = m.ascent; break;
        case VAlign::Middle:   baseline = (m.ascent - m.descent) / 2.0; break;
        case VAlign::Bottom:   baseline = -m.descent; break;
    }

    const int anchorX = roundToInt(x);
    const int anchorY = roundToInt(y);
    const int baseY = roundToInt(baseline);

    const std::vector<int> cps = decodeUtf8(text);
    double pen = 0.0;
    for (std::size_t k = 0; k < cps.size(); ++k) {
        const Glyph& g = font.glyph(cps[k], pixelSize);
        const int gx = roundToInt(startX + pen) + g.offsetX;
        const int gy = baseY + g.offsetY;

        for (int row = 0; row < g.height; ++row) {
            for (int col = 0; col < g.width; ++col) {
                const int coverage = g.coverage[static_cast<std::size_t>(row) * static_cast<std::size_t>(g.width) +
                                                static_cast<std::size_t>(col)];
                if (coverage == 0) continue;
                const int alpha = coverage * color.a / 255;
                if (alpha == 0) continue;

                const int tx = gx + col;    // pixel position in the text's own frame
                const int ty = gy + row;
                int sx = 0, sy = 0;
                if (rotation == TextRotation::None) {
                    sx = anchorX + tx;
                    sy = anchorY + ty;
                } else {
                    // 90 degrees anticlockwise: text runs upward, letter tops face left.
                    sx = anchorX + ty;
                    sy = anchorY - tx - 1;
                }
                ctx.setPixel(sx, sy, Color(color.r, color.g, color.b, static_cast<std::uint8_t>(alpha)));
            }
        }

        pen += g.advance;
        if (k + 1 < cps.size()) pen += font.kerning(cps[k], cps[k + 1], pixelSize);
    }
}

}  // namespace gfx
