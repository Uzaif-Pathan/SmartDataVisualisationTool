#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace gfx {

// Vertical measurements of a font at a given pixel size.
// ascent  = distance from the baseline UP to the top of the tallest letters.
// descent = distance from the baseline DOWN to the bottom of letters like g, y.
// ascent + descent always equals the pixel size.
struct FontMetrics {
    double ascent = 0.0;
    double descent = 0.0;
    double lineHeight = 0.0;
};

// One character, drawn into a small greyscale picture.
struct Glyph {
    int width = 0;                    // size of the coverage picture
    int height = 0;
    int offsetX = 0;                  // where its top-left sits, relative to the
    int offsetY = 0;                  // pen position on the baseline (y grows down)
    double advance = 0.0;             // how far the pen moves after this character
    std::vector<std::uint8_t> coverage;   // width*height values, 0 (empty) to 255 (solid)
};

// Loads a .ttf font file and hands out glyph pictures (cached for speed).
class FontManager {
public:
    FontManager();
    ~FontManager();
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    // Returns false (and keeps the previous font) if the file is missing or invalid.
    bool loadFromFile(const std::string& path);

    // Looks for DejaVuSans.ttf: next to the program, in the project's
    // assets/fonts folder, then in the standard system font folder.
    bool loadDefault();

    bool isLoaded() const;
    const std::string& loadedPath() const;

    // A shared font, loaded automatically the first time it is asked for.
    static FontManager& defaultFont();

    // Path where the default font was found, or "" if it was not found.
    static std::string findDefaultFontPath();

    // pixelSize is limited to the range 1..500.
    FontMetrics metrics(double pixelSize) const;

    // Unknown characters are replaced by '?'.  Returns an empty glyph if no
    // font is loaded.  The returned reference stays valid while the
    // FontManager exists.
    const Glyph& glyph(int codepoint, double pixelSize);

    // Extra spacing (usually 0 or negative) between two specific letters.
    double kerning(int codepoint1, int codepoint2, double pixelSize) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace gfx
