#include "graphics/charts/ChartText.h"

#include <cmath>

#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

namespace gfx {
namespace {

// Removes the last UTF-8 character (all of its bytes).
void popLastChar(std::string& s) {
    while (!s.empty() && (static_cast<unsigned char>(s.back()) & 0xC0) == 0x80) s.pop_back();
    if (!s.empty()) s.pop_back();
}

}  // namespace

std::string fitText(const std::string& text, double pixelSize, double maxWidth) {
    if (!std::isfinite(maxWidth) || maxWidth <= 0.0) return std::string();
    if (!std::isfinite(pixelSize) || pixelSize <= 0.0) return std::string();

    FontManager& font = FontManager::defaultFont();
    if (!font.isLoaded()) return text;       // cannot measure, so leave it alone
    if (measureText(font, text, pixelSize).width <= maxWidth) return text;

    const std::string ellipsis = "\xE2\x80\xA6";
    if (measureText(font, ellipsis, pixelSize).width > maxWidth) return std::string();

    std::string s = text;
    if (s.size() > 200) {                     // keep the loop below short for huge strings
        s.resize(200);
        popLastChar(s);
    }
    while (!s.empty()) {
        popLastChar(s);
        const std::string candidate = s + ellipsis;
        if (measureText(font, candidate, pixelSize).width <= maxWidth) return candidate;
    }
    return ellipsis;
}

}  // namespace gfx
