#include <cmath>
#include <iostream>
#include <limits>
#include <string>

#include "graphics/Renderer.h"
#include "graphics/primitives/Circle.h"
#include "graphics/primitives/Line.h"
#include "graphics/primitives/Text.h"
#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

using namespace gfx;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

struct Ink {
    bool any = false;
    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    int count = 0;
};

// Finds every pixel that is not plain white.
static Ink inkBounds(const Canvas& c) {
    Ink ink;
    for (int y = 0; y < c.height(); ++y) {
        for (int x = 0; x < c.width(); ++x) {
            if (c.getPixel(x, y) == colors::White) continue;
            ++ink.count;
            if (!ink.any) {
                ink.any = true;
                ink.minX = ink.maxX = x;
                ink.minY = ink.maxY = y;
            } else {
                if (x < ink.minX) ink.minX = x;
                if (x > ink.maxX) ink.maxX = x;
                if (y < ink.minY) ink.minY = y;
                if (y > ink.maxY) ink.maxY = y;
            }
        }
    }
    return ink;
}

static Text makeText(const std::string& s, double x, double y, double size,
                     HAlign h = HAlign::Left, VAlign v = VAlign::Baseline,
                     TextRotation rot = TextRotation::None,
                     Color color = colors::Black) {
    Text t;
    t.content = s;
    t.x = x;
    t.y = y;
    t.size = size;
    t.hAlign = h;
    t.vAlign = v;
    t.rotation = rot;
    t.color = color;
    return t;
}

static void testUtf8() {
    auto a = decodeUtf8("A");
    check(a.size() == 1 && a[0] == 'A', "utf8: plain ASCII");
    auto e = decodeUtf8(std::string("\xC3\xA9"));
    check(e.size() == 1 && e[0] == 0xE9, "utf8: 2-byte character (e-acute)");
    auto euro = decodeUtf8(std::string("\xE2\x82\xAC"));
    check(euro.size() == 1 && euro[0] == 0x20AC, "utf8: 3-byte character (euro sign)");
    auto emoji = decodeUtf8(std::string("\xF0\x9F\x98\x80"));
    check(emoji.size() == 1 && emoji[0] == 0x1F600, "utf8: 4-byte character");
    auto bad = decodeUtf8(std::string("\xFF"));
    check(bad.size() == 1 && bad[0] == '?', "utf8: invalid byte becomes '?'");
    auto cut = decodeUtf8(std::string("\xE2\x82"));
    check(cut.size() == 1 && cut[0] == '?', "utf8: truncated character becomes '?'");
    auto nl = decodeUtf8("a\nb");
    check(nl.size() == 3 && nl[1] == ' ', "utf8: newline becomes a space");
}

static void testMetrics(FontManager& f) {
    const FontMetrics m = f.metrics(20);
    check(std::fabs(m.ascent + m.descent - 20.0) < 0.01, "ascent + descent = font size (20)");
    check(m.ascent > m.descent && m.descent > 0, "ascent > descent > 0");

    const double w5 = measureText(f, "Hello", 20).width;
    check(w5 > 30 && w5 < 70, "'Hello' at 20px is between 30 and 70 px wide");
    check(measureText(f, "Hello World", 20).width > w5, "longer text is wider");
    check(measureText(f, "", 20).width == 0, "empty text has width 0");
    check(measureText(f, "i", 20).width < measureText(f, "W", 20).width, "'i' is narrower than 'W'");
    const double w40 = measureText(f, "Hello", 40).width;
    check(std::fabs(w40 / w5 - 2.0) < 0.05, "doubling the size doubles the width");
    check(measureText(f, "Hello", -3).width == 0 &&
          measureText(f, "Hello", std::numeric_limits<double>::quiet_NaN()).width == 0,
          "invalid size gives width 0");
}

static void testDrawing(FontManager& f) {
    // --- Left / baseline ---
    {
        Renderer r(200, 60);
        Text t = makeText("Hello", 20, 40, 20);
        t.font = &f;
        r.draw(t);
        const Ink ink = inkBounds(r.canvas());
        const TextMetrics m = t.measure();
        check(ink.count > 30, "text draws a reasonable number of pixels");
        check(ink.minX >= 18 && ink.maxX <= 20 + m.width + 2, "left-aligned: ink stays within the text width");
        check(ink.minY >= 40 - m.ascent - 2 && ink.maxY <= 40 + m.descent + 2,
              "baseline-aligned: ink stays between ascent and descent");
    }
    // --- Center ---
    {
        Renderer r(200, 60);
        Text t = makeText("Hello", 100, 40, 20, HAlign::Center);
        t.font = &f;
        r.draw(t);
        const Ink ink = inkBounds(r.canvas());
        check(std::abs((ink.minX + ink.maxX) / 2.0 - 100.0) <= 3.0, "centered: ink is centred on x = 100");
    }
    // --- Right ---
    {
        Renderer r(200, 60);
        Text t = makeText("Hello", 150, 40, 20, HAlign::Right);
        t.font = &f;
        r.draw(t);
        const Ink ink = inkBounds(r.canvas());
        check(ink.maxX <= 150 && ink.maxX >= 146, "right-aligned: text ends at x = 150");
    }
    // --- Top ---
    {
        Renderer r(200, 80);
        Text t = makeText("Hello", 20, 10, 20, HAlign::Left, VAlign::Top);
        t.font = &f;
        r.draw(t);
        const Ink ink = inkBounds(r.canvas());
        check(ink.minY >= 10 && ink.minY <= 18, "top-aligned: text starts just below y = 10");
    }
    // --- Bottom ---
    {
        Renderer r(200, 80);
        Text t = makeText("Hello", 20, 70, 20, HAlign::Left, VAlign::Bottom);
        t.font = &f;
        r.draw(t);
        const Ink ink = inkBounds(r.canvas());
        check(ink.maxY <= 70 && ink.maxY >= 60, "bottom-aligned: text ends just above y = 70");
    }
    // --- Rotated, centred ---
    {
        Renderer r(100, 200);
        Text t = makeText("Hello", 50, 100, 20, HAlign::Center, VAlign::Middle, TextRotation::Up);
        t.font = &f;
        r.draw(t);
        const Ink ink = inkBounds(r.canvas());
        const int inkW = ink.maxX - ink.minX + 1;
        const int inkH = ink.maxY - ink.minY + 1;
        check(inkH > inkW, "rotated text is taller than it is wide");
        check(std::abs((ink.minX + ink.maxX) / 2.0 - 50.0) <= 3.0, "rotated + middle: centred on x = 50");
        check(std::abs((ink.minY + ink.maxY) / 2.0 - 100.0) <= 4.0, "rotated + centre: centred on y = 100");
    }
    // --- Rotated: reads upward, letter tops face left ---
    {
        Renderer r(100, 200);
        Text t = makeText("Hello", 50, 150, 20, HAlign::Left, VAlign::Baseline, TextRotation::Up);
        t.font = &f;
        r.draw(t);
        const Ink ink = inkBounds(r.canvas());
        check(ink.maxY <= 151 && ink.minY < 130, "rotated: text starts at the anchor and runs upward");
        check(ink.maxX <= 55 && ink.minX >= 34, "rotated: letter tops face left, baseline on the anchor");
    }
    // --- Clipping ---
    {
        Renderer r(200, 60);
        r.context().setClip(0, 0, 50, 60);
        Text t = makeText("Hello World", 20, 40, 20);
        t.font = &f;
        r.draw(t);
        const Ink ink = inkBounds(r.canvas());
        check(ink.count > 0 && ink.maxX < 50, "text is cut off at the clip edge");
    }
    // --- Translucent colour ---
    {
        Renderer r(200, 60);
        Text t = makeText("Hello", 20, 40, 20, HAlign::Left, VAlign::Baseline,
                          TextRotation::None, Color(0, 0, 0, 128));
        t.font = &f;
        r.draw(t);
        bool light = true;
        for (int y = 0; y < 60; ++y)
            for (int x = 0; x < 200; ++x) {
                const Color p = r.canvas().getPixel(x, y);
                if (p != colors::White && p.r < 127) light = false;
            }
        check(light, "50%-transparent black text blends to grey, never solid black");
    }
    // --- Missing character ---
    {
        Renderer r(100, 60);
        Text t = makeText(std::string("\xF0\x9F\x98\x80"), 20, 40, 24);
        t.font = &f;
        r.draw(t);
        check(inkBounds(r.canvas()).count > 0, "a character missing from the font is drawn as '?'");
    }
}

static void testEdgeCases(FontManager& f) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();

    Renderer r(100, 100);
    Text t1 = makeText("Hi", nan, 50, 20);   t1.font = &f;
    Text t2 = makeText("Hi", 50, inf, 20);   t2.font = &f;
    Text t3 = makeText("Hi", 20, 50, 0);     t3.font = &f;
    Text t4 = makeText("Hi", 20, 50, -5);    t4.font = &f;
    Text t5 = makeText("Hi", 20, 50, nan);   t5.font = &f;
    Text t6 = makeText("", 20, 50, 20);      t6.font = &f;
    r.draw(t1); r.draw(t2); r.draw(t3); r.draw(t4); r.draw(t5); r.draw(t6);
    check(!inkBounds(r.canvas()).any, "NaN / infinite / zero size / empty text draw nothing");

    Text huge = makeText("Hi", 50, 50, 1e9);
    huge.font = &f;
    r.draw(huge);
    check(true, "absurd font size (1e9) finishes without hanging");

    Renderer r2(100, 100);
    Text tiny = makeText("Hi", 50, 50, 1e-9);
    tiny.font = &f;
    r2.draw(tiny);
    check(true, "tiny font size does not crash");

    Renderer r3(100, 100);
    Text longText = makeText(std::string(10000, 'W'), 0, 50, 12);
    longText.font = &f;
    r3.draw(longText);
    check(inkBounds(r3.canvas()).any, "10000-character string draws (and is clipped) without trouble");

    // A font that failed to load must not crash anything.
    FontManager broken;
    check(!broken.loadFromFile("/no/such/font.ttf"), "loading a missing font file returns false");
    check(!broken.isLoaded(), "a failed load leaves the FontManager empty");
    Renderer r4(100, 100);
    Text t = makeText("Hi", 20, 50, 20);
    t.font = &broken;
    r4.draw(t);
    check(!inkBounds(r4.canvas()).any && t.measure().width == 0, "no font: draws nothing, width 0");
}

static void marker(Renderer& r, double x, double y) {
    Circle c;
    c.cx = x;
    c.cy = y;
    c.radius = 3;
    c.fill = colors::Red;
    r.draw(c);
}

static void drawDemo() {
    Renderer r(800, 520, Color(250, 250, 250));

    r.draw(makeText("Smart Data Visualisation", 400, 12, 30, HAlign::Center, VAlign::Top));

    // Different sizes
    const double sizes[] = {10, 14, 20, 28, 40};
    const double baselines[] = {70, 92, 118, 152, 196};
    for (int i = 0; i < 5; ++i)
        r.draw(makeText("Sample Text 123", 20, baselines[i], sizes[i]));

    // Alignment around a red anchor line
    r.draw(Line{600, 60, 600, 200, colors::Red, 1});
    r.draw(makeText("Left", 600, 90, 20, HAlign::Left));
    r.draw(makeText("Center", 600, 130, 20, HAlign::Center));
    r.draw(makeText("Right", 600, 170, 20, HAlign::Right));
    marker(r, 600, 90);
    marker(r, 600, 130);
    marker(r, 600, 170);

    // Special characters (the ? shows what a missing character looks like)
    r.draw(makeText("Temp 25\xC2\xB0" "C  caf\xC3\xA9  price \xE2\x82\xB9" "500", 20, 245, 18));
    r.draw(makeText("Missing glyph: \xF0\x9F\x98\x80", 20, 275, 18));

    // Translucent overlap
    r.draw(makeText("Overlap", 20, 335, 48, HAlign::Left, VAlign::Baseline,
                    TextRotation::None, Color(200, 0, 0, 110)));
    r.draw(makeText("Overlap", 60, 350, 48, HAlign::Left, VAlign::Baseline,
                    TextRotation::None, Color(0, 0, 200, 110)));

    // Rotated y-axis title
    r.draw(makeText("Total Sales (\xE2\x82\xB9" ")", 14, 440, 16, HAlign::Center, VAlign::Middle,
                    TextRotation::Up));

    // Rotated sample with its anchor
    r.draw(makeText("Rotated", 700, 300, 22, HAlign::Center, VAlign::Middle, TextRotation::Up));
    marker(r, 700, 300);

    // A miniature chart frame: this is how axis labels will be placed
    r.draw(Line{250, 490, 550, 490, colors::Black, 1});
    r.draw(Line{250, 370, 250, 490, colors::Black, 1});
    const char* xLabels[] = {"0", "50", "100", "150"};
    for (int i = 0; i < 4; ++i) {
        const double x = 250 + i * 100;
        r.draw(Line{x, 490, x, 495, colors::Black, 1});
        r.draw(makeText(xLabels[i], x, 498, 12, HAlign::Center, VAlign::Top));
    }
    const char* yLabels[] = {"0", "5", "10"};
    for (int i = 0; i < 3; ++i) {
        const double y = 490 - i * 60;
        r.draw(Line{245, y, 250, y, colors::Black, 1});
        r.draw(makeText(yLabels[i], 242, y, 12, HAlign::Right, VAlign::Middle));
    }

    std::cout << (r.savePng("output/test_text.png")
                      ? "\nDemo saved: output/test_text.png\n"
                      : "\nCould not save output/test_text.png\n");
}

int main() {
    FontManager& font = FontManager::defaultFont();
    check(font.isLoaded(), "default font loads (" + font.loadedPath() + ")");
    if (!font.isLoaded()) {
        std::cout << "\nFont not found. Check that assets/fonts/DejaVuSans.ttf exists\n"
                     "(run this from the project folder) and repeat Step 10.\n";
        return 1;
    }

    testUtf8();
    testMetrics(font);
    testDrawing(font);
    testEdgeCases(font);
    drawDemo();

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
