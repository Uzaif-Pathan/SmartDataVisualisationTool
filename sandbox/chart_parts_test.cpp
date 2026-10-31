#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "graphics/Renderer.h"
#include "graphics/Viewport.h"
#include "graphics/charts/Axis.h"
#include "graphics/charts/ChartText.h"
#include "graphics/charts/Grid.h"
#include "graphics/charts/Legend.h"
#include "graphics/charts/Title.h"
#include "graphics/layout/AxisLayout.h"
#include "graphics/layout/ChartLayout.h"
#include "graphics/primitives/Rectangle.h"
#include "graphics/text/FontManager.h"
#include "graphics/text/TextRenderer.h"

using namespace gfx;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

static const double kNaN = std::numeric_limits<double>::quiet_NaN();

struct Ink {
    bool any = false;
    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    int count = 0;
};

// Every pixel that is not plain white.
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
                ink.minX = std::min(ink.minX, x);
                ink.maxX = std::max(ink.maxX, x);
                ink.minY = std::min(ink.minY, y);
                ink.maxY = std::max(ink.maxY, y);
            }
        }
    }
    return ink;
}

// Number of non-white pixels inside a box (inclusive, clipped to the canvas).
static int inkInBox(const Canvas& c, int x0, int y0, int x1, int y1) {
    int n = 0;
    for (int y = std::max(0, y0); y <= std::min(c.height() - 1, y1); ++y)
        for (int x = std::max(0, x0); x <= std::min(c.width() - 1, x1); ++x)
            if (c.getPixel(x, y) != colors::White) ++n;
    return n;
}

static bool hasQuestionMark(const std::string& s) {
    for (int cp : decodeUtf8(s)) if (cp == '?') return true;
    return false;
}

// ---------------------------------------------------------------- fitText
static void testFitText() {
    check(fitText("Sales", 12, 200) == "Sales", "fitText: short text is unchanged");

    const std::string longText = "A very long category name that cannot possibly fit";
    const std::string cut = fitText(longText, 12, 80);
    check(!cut.empty() && cut != longText, "fitText: long text is shortened");
    check(measureText(FontManager::defaultFont(), cut, 12).width <= 80.0, "fitText: result fits the width");
    check(cut.size() >= 3 && cut.compare(cut.size() - 3, 3, "\xE2\x80\xA6") == 0, "fitText: ends with '...'");

    const std::string utf8 = fitText("caf\xC3\xA9 caf\xC3\xA9 caf\xC3\xA9 caf\xC3\xA9 caf\xC3\xA9", 14, 60);
    check(!utf8.empty() && !hasQuestionMark(utf8), "fitText: never cuts a UTF-8 character in half");

    check(fitText("Hello", 12, 0).empty() && fitText("Hello", 12, -5).empty() &&
          fitText("Hello", 12, kNaN).empty(), "fitText: bad width gives empty text");
    check(fitText("Hello", 0, 100).empty() && fitText("Hello", kNaN, 100).empty(),
          "fitText: bad font size gives empty text");
    check(fitText("Hello", 12, 3).empty(), "fitText: too narrow even for '...' gives empty text");

    const std::string huge = fitText(std::string(100000, 'W'), 12, 100);
    check(!huge.empty() && measureText(FontManager::defaultFont(), huge, 12).width <= 100.0,
          "fitText: a 100000-character string is handled quickly");
}

// ----------------------------------------------------------------- Axis
static ChartLayout simpleLayout() {
    LayoutOptions o;
    o.hasTitle = o.hasXAxisTitle = o.hasYAxisTitle = true;
    o.yTickLabelWidth = 30;
    return computeChartLayout(400, 300, o);
}

static void testAxis() {
    const ChartLayout L = simpleLayout();
    const PixelRect area{60, 30, 300, 200};      // right = 360, bottom = 230
    const Viewport vp(area, 0, 10, 0, 100);
    const AxisScale xs = niceScale(0, 10, 6);    // 0,2,...,10
    const AxisScale ys = niceScale(0, 100, 6);   // 0,20,...,100
    AxisStyle style;

    {
        Renderer r(400, 300);
        drawPlotFrame(r, area, style);
        check(inkInBox(r.canvas(), 58, 28, 62, 32) > 0 && inkInBox(r.canvas(), 358, 228, 362, 232) > 0,
              "drawPlotFrame: draws the corners");
        check(inkInBox(r.canvas(), 100, 100, 300, 200) == 0, "drawPlotFrame: inside stays empty");
        Renderer r2(400, 300);
        drawPlotFrame(r2, PixelRect{60, 30, 0, 0}, style);
        check(!inkBounds(r2.canvas()).any, "drawPlotFrame: empty area draws nothing");
    }
    {
        Renderer r(400, 300);
        drawXAxisTicks(r, vp, xs, L, style);
        const int px = static_cast<int>(vp.mapX(4));     // 180
        check(inkInBox(r.canvas(), px - 1, 231, px + 1, 232) > 0, "x ticks: tick mark below the plot area");
        check(inkInBox(r.canvas(), px - 12, 234, px + 12, 262) > 0, "x ticks: label under the tick mark");
        check(inkInBox(r.canvas(), 0, 0, 399, 229) == 0, "x ticks: nothing drawn inside or above the plot area");
    }
    {
        Renderer r(400, 300);
        drawYAxisTicks(r, vp, ys, L, style);
        const int py = static_cast<int>(vp.mapY(40));    // 150
        check(inkInBox(r.canvas(), 58, py - 1, 59, py + 1) > 0, "y ticks: tick mark left of the plot area");
        check(inkInBox(r.canvas(), 20, py - 8, 52, py + 8) > 0, "y ticks: label left of the tick mark");
        check(inkInBox(r.canvas(), 61, 0, 399, 299) == 0, "y ticks: nothing drawn inside the plot area");
    }
    {
        // 50 ticks squeezed into 100 px: labels must be thinned out, not crash.
        Renderer r(400, 300);
        const Viewport crowded(PixelRect{60, 30, 100, 100}, 0, 49, 0, 49);
        const AxisScale many = niceScale(0, 49, 50);
        drawXAxisTicks(r, crowded, many, L, style);
        drawYAxisTicks(r, crowded, many, L, style);
        check(inkBounds(r.canvas()).any, "crowded ticks: drawn without trouble");
    }
    {
        Renderer r(400, 300);
        AxisScale bad = xs;
        bad.ticks.push_back(kNaN);
        drawXAxisTicks(r, vp, bad, L, style);
        drawXAxisTicks(r, Viewport(PixelRect{60, 30, 0, 0}, 0, 1, 0, 1), xs, L, style);
        AxisScale empty;
        empty.ticks.clear();
        drawYAxisTicks(r, vp, empty, L, style);
        check(true, "ticks: NaN tick, empty area and empty scale do not crash");
    }
    {
        Renderer r(400, 300);
        const std::vector<double> centers = {110, 210, 310};
        drawCategoryLabels(r, centers, {"Laptop", "Phone", "Tablet"}, 100, area, L, style);
        check(inkInBox(r.canvas(), 90, 234, 130, 262) > 0 && inkInBox(r.canvas(), 290, 234, 330, 262) > 0,
              "category labels: a label under every category");
    }
    {
        Renderer r(400, 300);
        drawCategoryLabels(r, {210}, {"AVeryLongCategoryNameThatCannotFit"}, 60, area, L, style);
        const Ink ink = inkBounds(r.canvas());
        check(ink.any && ink.minX >= 210 - 31 && ink.maxX <= 210 + 31,
              "category labels: a long name is shortened to its 60 px slot");
    }
    {
        Renderer r(400, 300);
        std::vector<double> centers;
        std::vector<std::string> names;
        for (int i = 0; i < 40; ++i) { centers.push_back(60 + i * 7.5); names.push_back("Cat" + std::to_string(i)); }
        drawCategoryLabels(r, centers, names, 7.5, area, L, style);
        check(inkBounds(r.canvas()).any, "category labels: 40 crowded categories are thinned out, no crash");

        Renderer r2(400, 300);
        drawCategoryLabels(r2, {100, 200, 300}, {"A", "B"}, 100, area, L, style);   // mismatched lists
        drawCategoryLabels(r2, {100}, {"A"}, 0, area, L, style);
        drawCategoryLabels(r2, {100}, {"A"}, kNaN, area, L, style);
        drawCategoryLabels(r2, {kNaN}, {"A"}, 50, area, L, style);
        check(true, "category labels: mismatched lists, zero/NaN slot and NaN centre do not crash");
    }
}

// ----------------------------------------------------------------- Grid
static void testGrid() {
    const PixelRect area{60, 30, 300, 200};
    const Viewport vp(area, 0, 10, 0, 100);
    const Color grid(200, 200, 200);

    {
        Renderer r(400, 300);
        drawHorizontalGrid(r, vp, {20, 40}, grid);      // y = 190 and 150
        check(inkInBox(r.canvas(), 100, 190, 101, 190) > 0 && inkInBox(r.canvas(), 100, 150, 101, 150) > 0,
              "horizontal grid: a line at each tick");
        check(inkInBox(r.canvas(), 0, 190, 57, 190) == 0 && inkInBox(r.canvas(), 363, 190, 399, 190) == 0,
              "horizontal grid: lines stay inside the plot area");
    }
    {
        Renderer r(400, 300);
        drawVerticalGrid(r, vp, {2, 4}, grid);          // x = 120 and 180
        check(inkInBox(r.canvas(), 120, 100, 120, 101) > 0 && inkInBox(r.canvas(), 180, 100, 180, 101) > 0,
              "vertical grid: a line at each tick");
        check(inkInBox(r.canvas(), 120, 0, 120, 27) == 0 && inkInBox(r.canvas(), 120, 233, 120, 299) == 0,
              "vertical grid: lines stay inside the plot area");
    }
    {
        Renderer r(400, 300);
        drawHorizontalGrid(r, vp, {500, -500, kNaN}, grid);
        drawVerticalGrid(r, vp, {500, -500, kNaN}, grid);
        check(!inkBounds(r.canvas()).any, "grid: values outside the range or NaN draw nothing");
        drawHorizontalGrid(r, Viewport(PixelRect{60, 30, 0, 0}, 0, 1, 0, 1), {0.5}, grid);
        check(true, "grid: empty plot area does not crash");
    }
    {
        Renderer r(400, 300);
        const Viewport both(area, -10, 10, -10, 10);    // zero lines at x = 210, y = 130
        drawZeroLines(r, both, Color(90, 90, 90));
        check(inkInBox(r.canvas(), 100, 130, 101, 130) > 0 && inkInBox(r.canvas(), 210, 60, 210, 61) > 0,
              "zero lines: drawn where 0 is inside the range");
        Renderer r2(400, 300);
        drawZeroLines(r2, Viewport(area, 1, 10, 1, 10), Color(90, 90, 90));
        check(!inkBounds(r2.canvas()).any, "zero lines: not drawn when 0 is outside the range");
    }
}

// ---------------------------------------------------------------- Title
static void testTitles() {
    const ChartLayout L = simpleLayout();     // 400 x 300 with all three titles
    {
        Renderer r(400, 300);
        drawTitle(r, L, "Sales by Product", 20, colors::Black);
        const Ink ink = inkBounds(r.canvas());
        check(ink.any && ink.minY >= 0 && ink.maxY <= L.plotArea.top, "title: sits in the top margin");
        check(std::abs((ink.minX + ink.maxX) / 2.0 - L.titleX) <= 4.0, "title: centred over the plot area");
    }
    {
        Renderer r(400, 300);
        drawTitle(r, L, std::string(200, 'W'), 20, colors::Black);
        const Ink ink = inkBounds(r.canvas());
        check(ink.any && ink.minX >= 0 && ink.maxX < 400, "title: a very long title is cut to fit the canvas");
    }
    {
        Renderer r(400, 300);
        drawTitle(r, L, "", 20, colors::Black);
        drawXAxisTitle(r, L, "", 14, colors::Black);
        drawYAxisTitle(r, L, "", 14, colors::Black);
        check(!inkBounds(r.canvas()).any, "titles: empty text draws nothing");
    }
    {
        Renderer r(400, 300);
        drawXAxisTitle(r, L, "Product", 14, colors::Black);
        const Ink ink = inkBounds(r.canvas());
        check(ink.any && ink.minY > L.plotArea.bottom() && ink.maxY <= 300, "x title: below the plot area, inside the canvas");
        check(std::abs((ink.minX + ink.maxX) / 2.0 - L.xTitleX) <= 4.0, "x title: centred under the plot area");
    }
    {
        Renderer r(400, 300);
        drawYAxisTitle(r, L, "Total Sales", 14, colors::Black);
        const Ink ink = inkBounds(r.canvas());
        check(ink.any && ink.minX >= 0 && ink.maxX < L.plotArea.left, "y title: left of the plot area, inside the canvas");
        check(ink.maxY - ink.minY > ink.maxX - ink.minX, "y title: reads bottom-to-top (taller than wide)");
        check(std::abs((ink.minY + ink.maxY) / 2.0 - L.yTitleY) <= 5.0, "y title: centred along the plot height");
    }
    {
        Renderer r(400, 300);
        drawYAxisTitle(r, L, std::string(300, 'W'), 14, colors::Black);
        const Ink ink = inkBounds(r.canvas());
        check(ink.any && ink.minY >= L.plotArea.top - 2 && ink.maxY <= L.plotArea.bottom() + 2,
              "y title: a very long title is cut to the plot height");
    }
}

// --------------------------------------------------------------- Legend
static void testLegend() {
    check(paletteColor(0) == paletteColor(10) && paletteColor(3) == paletteColor(13), "palette: wraps around after 10 colours");
    check(!(paletteColor(0) == paletteColor(1)), "palette: neighbouring colours differ");

    std::vector<LegendEntry> entries = {
        {"Laptop", paletteColor(0)}, {"Phone", paletteColor(1)}, {"Tablet", paletteColor(2)}};
    const double w = measureLegendWidth(entries, 12);
    check(w > 20 && w < 200, "measureLegendWidth: a sensible width");
    check(measureLegendWidth({}, 12) == 0 && measureLegendWidth(entries, 0) == 0 &&
          measureLegendWidth(entries, kNaN) == 0, "measureLegendWidth: 0 for empty list or bad size");
    std::vector<LegendEntry> wideEntries = {{std::string(200, 'W'), paletteColor(0)}};
    check(measureLegendWidth(wideEntries, 12) <= 12 * 0.9 + 6 + 160 + 4 + 1e-9, "measureLegendWidth: capped for very long labels");

    {
        Renderer r(400, 300);
        const PixelRect area{280, 30, 110, 150};
        drawLegend(r, area, entries, 12, colors::Black);
        const double rowH = 12 * 1.6;
        const int cx = static_cast<int>(area.left + 12 * 0.9 / 2.0);
        check(r.canvas().getPixel(cx, static_cast<int>(area.top + rowH / 2.0)) == paletteColor(0),
              "legend: first swatch has the first palette colour");
        check(r.canvas().getPixel(cx, static_cast<int>(area.top + rowH * 1.5)) == paletteColor(1),
              "legend: second swatch has the second palette colour");
        check(inkInBox(r.canvas(), 296, 30, 390, 50) > 0, "legend: a label is drawn next to the swatch");
        const Ink ink = inkBounds(r.canvas());
        check(ink.minX >= 280 && ink.maxX <= 391 && ink.minY >= 30 && ink.maxY <= 180, "legend: everything stays inside its area");
    }
    {
        Renderer r(400, 300);
        std::vector<LegendEntry> many;
        for (int i = 0; i < 20; ++i) many.push_back({"Series " + std::to_string(i), paletteColor(i)});
        const PixelRect area{280, 30, 110, 100};
        drawLegend(r, area, many, 12, colors::Black);
        const Ink ink = inkBounds(r.canvas());
        check(ink.any && ink.maxY <= area.bottom() + 1, "legend: 20 entries never spill below the area");
        check(inkInBox(r.canvas(), 280, 30 + 19 * 4, 390, 129) > 0, "legend: the last row shows '+N more'");
    }
    {
        Renderer r(400, 300);
        drawLegend(r, PixelRect{280, 30, 110, 100}, {}, 12, colors::Black);
        drawLegend(r, PixelRect{280, 30, 0, 0}, entries, 12, colors::Black);
        drawLegend(r, PixelRect{280, 30, 110, 10}, entries, 12, colors::Black);
        drawLegend(r, PixelRect{280, 30, 110, 100}, entries, kNaN, colors::Black);
        check(!inkBounds(r.canvas()).any, "legend: empty list, empty area, too-short area or bad size draw nothing");
    }
}

// ---------------------------------------------------------- Demo charts
// A bar chart built by hand from the new pieces.  This is the recipe the
// real BarChart will follow.
static void drawDemo(int width, int height, const std::string& filename) {
    Renderer r(width, height, Color(250, 250, 250));
    const std::vector<std::string> cats = {"Laptop", "Phone", "Tablet", "Accessories", "Monitor", "Printer"};
    const std::vector<double> vals = {520, 430, 210, 95, 300, 60};

    const double tickSize = std::max(9.0, std::min(14.0, height / 38.0));
    const double axisSize = tickSize + 2.0;
    const double titleSize = std::max(12.0, std::min(24.0, height / 22.0));

    const double maxVal = *std::max_element(vals.begin(), vals.end());
    const AxisScale ys = niceScale(0, maxVal, 6, true);

    std::vector<LegendEntry> legend;
    for (std::size_t i = 0; i < cats.size(); ++i) legend.push_back({cats[i], paletteColor(i)});

    double widest = 0.0;
    for (const std::string& s : tickLabels(ys))
        widest = std::max(widest, measureText(FontManager::defaultFont(), s, tickSize).width);

    LayoutOptions opt;
    opt.hasTitle = opt.hasXAxisTitle = opt.hasYAxisTitle = opt.hasLegend = true;
    opt.titleSize = titleSize;
    opt.axisTitleSize = axisSize;
    opt.tickLabelSize = tickSize;
    opt.yTickLabelWidth = widest;
    opt.legendWidth = measureLegendWidth(legend, tickSize);
    const ChartLayout L = computeChartLayout(width, height, opt);

    // x runs 0..6 so category i is centred on i + 0.5
    const Viewport vp(L.plotArea, 0, static_cast<double>(cats.size()), ys.min, ys.max);

    Rectangle bg;
    bg.x = L.plotArea.left; bg.y = L.plotArea.top;
    bg.width = L.plotArea.width; bg.height = L.plotArea.height;
    bg.fill = colors::White;
    r.draw(bg);

    drawHorizontalGrid(r, vp, ys.ticks, Color(225, 225, 225));

    vp.applyClip(r.context());
    for (std::size_t i = 0; i < cats.size(); ++i) {
        const double yTop = vp.mapY(vals[i]);
        const double yBase = vp.mapY(0.0);
        Rectangle bar;
        bar.x = vp.mapX(static_cast<double>(i) + 0.15);
        bar.y = std::min(yTop, yBase);
        bar.width = vp.mapX(static_cast<double>(i) + 0.85) - bar.x;
        bar.height = std::fabs(yBase - yTop);
        bar.fill = paletteColor(i);
        if (bar.height > 0.0) r.draw(bar);
    }
    r.context().clearClip();

    AxisStyle style;
    style.labelSize = tickSize;
    drawPlotFrame(r, L.plotArea, style);
    drawYAxisTicks(r, vp, ys, L, style);

    std::vector<double> centers;
    for (std::size_t i = 0; i < cats.size(); ++i) centers.push_back(vp.mapX(static_cast<double>(i) + 0.5));
    drawCategoryLabels(r, centers, cats, vp.mapX(1.0) - vp.mapX(0.0), L.plotArea, L, style);

    drawTitle(r, L, "Sales by Product", titleSize, Color(20, 20, 20));
    drawXAxisTitle(r, L, "Product", axisSize, Color(40, 40, 40));
    drawYAxisTitle(r, L, "Total Sales", axisSize, Color(40, 40, 40));
    drawLegend(r, L.legendArea, legend, tickSize, Color(40, 40, 40));

    std::cout << (r.savePng(filename) ? "Demo saved: " : "Could not save: ") << filename << "\n";
}

int main() {
    if (!FontManager::defaultFont().isLoaded()) {
        std::cout << "Font not found: check assets/fonts/DejaVuSans.ttf (run from the project folder).\n";
        return 1;
    }
    testFitText();
    testAxis();
    testGrid();
    testTitles();
    testLegend();

    std::cout << "\n";
    drawDemo(800, 500, "output/test_parts_large.png");
    drawDemo(320, 240, "output/test_parts_small.png");

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
