#include "graphics/text/FontManager.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <utility>

// This is the ONE place that compiles the font code from stb.
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

namespace gfx {
namespace {

double clampSize(double s) {
    if (!(s >= 1.0)) return 1.0;      // also catches NaN
    if (s > 500.0) return 500.0;
    return s;
}

}  // namespace

struct FontManager::Impl {
    std::vector<unsigned char> data;   // the whole font file
    stbtt_fontinfo info{};             // stb keeps a pointer into `data`
    bool loaded = false;
    std::string path;
    std::map<std::pair<int, long>, Glyph> cache;   // (character, size*16) -> picture
};

FontManager::FontManager() : impl_(std::make_unique<Impl>()) {}
FontManager::~FontManager() = default;

bool FontManager::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)),
                                     std::istreambuf_iterator<char>());
    if (bytes.size() < 12) return false;

    auto next = std::make_unique<Impl>();
    next->data = std::move(bytes);
    const int offset = stbtt_GetFontOffsetForIndex(next->data.data(), 0);
    if (offset < 0) return false;
    if (!stbtt_InitFont(&next->info, next->data.data(), offset)) return false;
    next->loaded = true;
    next->path = path;
    impl_ = std::move(next);
    return true;
}

std::string FontManager::findDefaultFontPath() {
    namespace fs = std::filesystem;
    const char* kName = "DejaVuSans.ttf";
    std::vector<fs::path> candidates;

    std::error_code ec;
    const fs::path exe = fs::read_symlink("/proc/self/exe", ec);   // where the program lives
    if (!ec) {
        const fs::path dir = exe.parent_path();
        candidates.push_back(dir / "assets" / "fonts" / kName);
        candidates.push_back(dir / ".." / "assets" / "fonts" / kName);
        candidates.push_back(dir / ".." / ".." / "assets" / "fonts" / kName);
    }
    candidates.push_back(fs::path("assets") / "fonts" / kName);       // current folder
    candidates.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");   // system copy

    for (const fs::path& c : candidates) {
        std::error_code e2;
        if (fs::is_regular_file(c, e2)) return c.string();
    }
    return std::string();
}

bool FontManager::loadDefault() {
    const std::string path = findDefaultFontPath();
    if (path.empty()) return false;
    return loadFromFile(path);
}

bool FontManager::isLoaded() const { return impl_->loaded; }

const std::string& FontManager::loadedPath() const { return impl_->path; }

FontManager& FontManager::defaultFont() {
    static FontManager font;
    static bool tried = false;
    if (!tried) {
        tried = true;
        font.loadDefault();
    }
    return font;
}

FontMetrics FontManager::metrics(double pixelSize) const {
    FontMetrics m;
    if (!impl_->loaded) return m;
    const float scale = stbtt_ScaleForPixelHeight(&impl_->info, static_cast<float>(clampSize(pixelSize)));
    int asc = 0, desc = 0, gap = 0;
    stbtt_GetFontVMetrics(&impl_->info, &asc, &desc, &gap);
    m.ascent = asc * scale;
    m.descent = -desc * scale;       // stb reports descent as a negative number
    m.lineHeight = (asc - desc + gap) * scale;
    return m;
}

const Glyph& FontManager::glyph(int codepoint, double pixelSize) {
    static const Glyph empty;
    if (!impl_->loaded) return empty;

    const long sizeKey = std::lround(clampSize(pixelSize) * 16.0);
    const auto key = std::make_pair(codepoint, sizeKey);
    const auto found = impl_->cache.find(key);
    if (found != impl_->cache.end()) return found->second;

    const stbtt_fontinfo* info = &impl_->info;
    int cp = codepoint;
    if (stbtt_FindGlyphIndex(info, cp) == 0) cp = '?';   // character not in this font

    const float scale = stbtt_ScaleForPixelHeight(info, static_cast<float>(sizeKey / 16.0));

    Glyph g;
    int advance = 0, leftBearing = 0;
    stbtt_GetCodepointHMetrics(info, cp, &advance, &leftBearing);
    g.advance = advance * scale;

    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    stbtt_GetCodepointBitmapBox(info, cp, scale, scale, &x0, &y0, &x1, &y1);
    g.width = x1 - x0;
    g.height = y1 - y0;
    g.offsetX = x0;
    g.offsetY = y0;
    if (g.width > 0 && g.height > 0) {
        g.coverage.assign(static_cast<std::size_t>(g.width) * static_cast<std::size_t>(g.height), 0);
        stbtt_MakeCodepointBitmap(info, g.coverage.data(), g.width, g.height, g.width,
                                  scale, scale, cp);
    } else {
        g.width = 0;
        g.height = 0;
    }
    return impl_->cache.emplace(key, std::move(g)).first->second;
}

double FontManager::kerning(int codepoint1, int codepoint2, double pixelSize) const {
    if (!impl_->loaded) return 0.0;
    const float scale = stbtt_ScaleForPixelHeight(&impl_->info, static_cast<float>(clampSize(pixelSize)));
    return stbtt_GetCodepointKernAdvance(&impl_->info, codepoint1, codepoint2) * scale;
}

}  // namespace gfx
