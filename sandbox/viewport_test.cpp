#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "graphics/Renderer.h"
#include "graphics/Viewport.h"
#include "graphics/layout/AxisLayout.h"
#include "graphics/layout/ChartLayout.h"
#include "graphics/primitives/Line.h"
#include "graphics/primitives/Point.h"
#include "graphics/primitives/Rectangle.h"
#include "graphics/primitives/Text.h"
#include "graphics/text/TextRenderer.h"

using namespace gfx;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

static bool near(double a, double b, double tol = 1e-6) { return std::fabs(a - b) <= tol; }

static const double kNaN = std::numeric_limits<double>::quiet_NaN();
static const double kInf = std::numeric_limits<double>::infinity();

// ---------------------------------------------------------------- Viewport
static void testViewport() {
    const PixelRect area{100, 50, 400, 300};

    Viewport v(area, 0, 10, 0, 100);
    check(!v.wasAdjusted(), "normal ranges are not adjusted");
    check(near(v.mapX(0), 100) && near(v.mapX(10), 500) && near(v.mapX(5), 300),
          "x: 0 -> left edge, 10 -> right edge, 5 -> middle");
    check(near(v.mapY(0), 350) && near(v.mapY(100), 50) && near(v.mapY(50), 200),
          "y: 0 -> bottom edge, 100 -> top edge, 50 -> middle");
    check(v.mapY(100) < v.mapY(0), "y axis is flipped (bigger values are higher up)");
    check(near(v.unmapX(v.mapX(3.7)), 3.7, 1e-9), "unmapX undoes mapX");
    check(near(v.unmapY(v.mapY(42.5)), 42.5, 1e-9), "unmapY undoes mapY");

    Viewport neg(area, -10, 10, -5, 5);
    check(near(neg.mapX(0), 300) && near(neg.mapX(-10), 100) && near(neg.mapX(10), 500),
          "negative x range maps correctly");
    check(near(neg.mapY(0), 200) && near(neg.mapY(5), 50) && near(neg.mapY(-5), 350),
          "negative y range maps correctly");

    Viewport zero(area, 5, 5, 7, 7);
    check(zero.wasAdjusted(), "zero range is repaired and reported");
    check(near(zero.mapX(5), 300) && near(zero.mapY(7), 200),
          "zero range: the single value sits in the middle");

    Viewport swapped(area, 10, 0, 100, 0);
    check(swapped.wasAdjusted() && near(swapped.mapX(0), 100) && near(swapped.mapX(10), 500) &&
          near(swapped.mapY(0), 350), "min > max is swapped");

    Viewport bad(area, kNaN, 5, 0, kInf);
    check(bad.wasAdjusted() && near(bad.mapX(0.5), 300) && near(bad.mapY(0.5), 200),
          "NaN / infinite ranges are replaced by 0..1");

    check(std::isnan(v.mapX(kNaN)) && std::isnan(v.mapY(kInf)), "NaN / infinite input gives NaN");
    check(v.mapX(1e300) == 1e7 && v.mapX(-1e300) == -1e7, "huge values are clamped to +/- 1e7 px");

    Viewport big(area, -1e300, 1e300, -1e300, 1e300);
    check(near(big.mapX(0), 300) && near(big.mapY(0), 200), "range of +/-1e300 works");
    Viewport tiny(area, 1e-12, 2e-12, 0, 1);
    check(near(tiny.mapX(1.5e-12), 300, 1e-3), "range of 1e-12 works");
    Viewport overflow(area, -1e308, 1e308, 0, 1);
    check(overflow.wasAdjusted() && near(overflow.mapX(0.5), 300),
          "range too big for a double falls back to 0..1");

    Viewport flat(PixelRect{100, 50, 0, 0}, 0, 10, 0, 10);
    check(near(flat.mapX(5), 100) && near(flat.mapY(5), 50) && near(flat.unmapX(100), 0),
          "zero-size area: everything maps to the corner, no crash");
    Viewport negArea(PixelRect{100, 50, -20, -20}, 0, 10, 0, 10);
    check(near(negArea.mapX(5), 100), "negative-size area is treated as zero size");

    // Clipping
    Renderer r(100, 80);
    Viewport clipVp(PixelRect{10, 10, 50, 40}, 0, 1, 0, 1);
    clipVp.applyClip(r.context());
    Rectangle everything;
    everything.x = 0; everything.y = 0; everything.width = 100; everything.height = 80;
    everything.fill = colors::Red;
    r.draw(everything);
    check(r.canvas().getPixel(10, 10) == colors::Red && r.canvas().getPixel(59, 49) == colors::Red,
          "applyClip: inside the plot area is drawn");
    check(r.canvas().getPixel(60, 30) == colors::White && r.canvas().getPixel(30, 50) == colors::White &&
          r.canvas().getPixel(9, 30) == colors::White && r.canvas().getPixel(30, 9) == colors::White,
          "applyClip: outside the plot area is not");
}

// -------------------------------------------------------------- AxisLayout
static void testAxis() {
    AxisScale a = niceScale(0, 97, 5);
    check(a.min == 0 && a.max == 100 && near(a.step, 20) && a.ticks.size() == 6 && a.decimals == 0,
          "niceScale(0, 97): 0..100 in steps of 20");

    AxisScale b = niceScale(0, 1, 5);
    check(b.min == 0 && near(b.max, 1) && near(b.step, 0.2) && b.ticks.size() == 6 && b.decimals == 1,
          "niceScale(0, 1): steps of 0.2, one decimal");
    check(near(b.ticks[3], 0.6, 1e-12), "the 4th tick is exactly 0.6 (no 0.6000000000000001)");

    AxisScale c = niceScale(-37, 62, 5);
    check(c.min == -40 && c.max == 80 && near(c.step, 20) && c.ticks.size() == 7,
          "niceScale(-37, 62): -40..80");
    check(c.ticks[2] == 0 && formatTick(c.ticks[2], c.decimals) == "0", "the middle tick is 0 and prints as '0'");

    AxisScale d = niceScale(5, 5, 5);
    check(d.adjusted && d.min < 5 && d.max > 5, "constant value: axis widened around it");

    AxisScale e = niceScale(120, 180, 5, true);
    check(e.min == 0 && e.max == 200, "includeZero: 120..180 becomes 0..200");
    AxisScale f = niceScale(-180, -120, 5, true);
    check(f.min == -200 && f.max == 0, "includeZero: all-negative data reaches 0 at the top");
    AxisScale g = niceScale(0, 0, 5, true);
    check(g.min == 0 && g.max == 1 && g.adjusted, "includeZero: all zeros gives 0..1");

    AxisScale h = niceScale(kNaN, 5);
    check(h.adjusted && h.min == 0 && h.max == 1 && !h.ticks.empty(), "NaN input: repaired to 0..1");
    AxisScale i = niceScale(-1e308, 1e308);
    check(i.adjusted, "range too big for a double: repaired");

    AxisScale j = niceScale(-1e300, 1e300);
    bool finite = j.ticks.size() >= 2;
    for (double t : j.ticks) finite = finite && std::isfinite(t);
    check(finite, "range of +/-1e300 gives finite ticks");

    check(niceScale(0, 1e9, 5).ticks.size() == 6, "niceScale(0, 1e9): 6 ticks");
    check(niceScale(0, 1e-9, 5).ticks.size() == 6, "niceScale(0, 1e-9): 6 ticks");
    check(niceScale(0, 100, 0).ticks.size() >= 2, "targetTicks = 0 is limited to at least 2 ticks");
    check(niceScale(0, 100, 1000).ticks.size() <= 100, "targetTicks = 1000 does not create hundreds of ticks");

    bool covers = true;
    const double data[][2] = {{0, 97}, {-37, 62}, {0.3, 0.7}, {1234, 5678}, {-0.002, 0.009}, {12, 12.5}};
    for (const auto& pair : data) {
        AxisScale s = niceScale(pair[0], pair[1], 5);
        covers = covers && s.min <= pair[0] && s.max >= pair[1];
    }
    check(covers, "the axis always covers the whole data range");

    check(formatTick(20, 0) == "20", "formatTick(20, 0) = '20'");
    check(formatTick(0.6, 1) == "0.6", "formatTick(0.6, 1) = '0.6'");
    check(formatTick(2.5, 1) == "2.5", "formatTick(2.5, 1) = '2.5'");
    check(formatTick(-0.0, 0) == "0", "negative zero prints as '0'");
    check(formatTick(-0.04, 1) == "0.0", "-0.04 with 1 decimal prints as '0' (not '-0.0')");
    check(formatTick(1234567, 0) == "1234567", "1234567 prints in full");
    check(formatTick(1e10, 0) == "1e+10", "10 billion uses scientific notation");
    check(formatTick(kNaN, 0).empty(), "NaN prints as empty text");

    std::vector<std::string> labels = tickLabels(b);
    check(labels.size() == 6 && labels[0] == "0.0" && labels[3] == "0.6" && labels[5] == "1.0",
          "tickLabels(0..1) = 0.0, 0.2, ... 1.0");
}

// ------------------------------------------------------------- ChartLayout
static void testLayout() {
    LayoutOptions full;
    full.hasTitle = full.hasXAxisTitle = full.hasYAxisTitle = true;
    full.yTickLabelWidth = 30;

    const ChartLayout L = computeChartLayout(800, 600, full);
    const PixelRect& p = L.plotArea;
    check(p.left > 0 && p.top > 0 && p.right() < 800 && p.bottom() < 600, "plot area is inside the canvas");
    check(p.width > 400 && p.height > 300, "plot area gets most of the canvas");
    check(L.titleY + full.titleSize <= p.top, "title fits above the plot area");
    check(L.xTitleY > p.bottom(), "x-axis title is below the plot area");
    check(L.yTitleX < p.left, "y-axis title is left of the plot area");

    const ChartLayout bare = computeChartLayout(800, 600, LayoutOptions());
    check(bare.plotArea.top < p.top && bare.plotArea.height > p.height,
          "no titles: the plot area is bigger");

    LayoutOptions wide = full;
    wide.yTickLabelWidth = 80;
    check(computeChartLayout(800, 600, wide).plotArea.left > p.left, "wider y labels push the plot area right");

    LayoutOptions withLegend = full;
    withLegend.hasLegend = true;
    withLegend.legendWidth = 120;
    const ChartLayout G = computeChartLayout(800, 600, withLegend);
    check(G.plotArea.width < p.width, "a legend makes the plot area narrower");
    check(G.legendArea.left >= G.plotArea.right() && G.legendArea.right() <= 800 + 1e-9 &&
          near(G.legendArea.width, 120), "legend area sits to the right of the plot, inside the canvas");

    check(computeChartLayout(1600, 1200, full).plotArea.width > p.width, "a bigger canvas gives a bigger plot");

    LayoutOptions everything = full;
    everything.hasLegend = true;
    everything.legendWidth = 200;
    const ChartLayout tiny = computeChartLayout(40, 30, everything);
    check(tiny.plotArea.width > 0 && tiny.plotArea.height > 0 && tiny.plotArea.left >= 0 &&
          tiny.plotArea.top >= 0 && tiny.plotArea.right() <= 40 + 1e-6 &&
          tiny.plotArea.bottom() <= 30 + 1e-6, "40x30 canvas: plot area survives and stays inside");
    const ChartLayout micro = computeChartLayout(5, 5, everything);
    check(micro.plotArea.width > 0 && micro.plotArea.right() <= 5 + 1e-6, "5x5 canvas: no crash, plot inside");

    check(computeChartLayout(0, 100, full).plotArea.width == 0 &&
          computeChartLayout(kNaN, 100, full).plotArea.width == 0 &&
          computeChartLayout(100, -5, full).plotArea.height == 0, "invalid canvas size gives an empty layout");

    LayoutOptions weird = full;
    weird.titleSize = kNaN;
    weird.yTickLabelWidth = -50;
    weird.tickLabelSize = kInf;
    const ChartLayout W = computeChartLayout(800, 600, weird);
    check(std::isfinite(W.plotArea.left) && std::isfinite(W.plotArea.width) && W.plotArea.width > 0,
          "NaN / negative / infinite options are tolerated");
}

// -------------------------------------------------------------- Demo charts
static Text makeText(const std::string& s, double x, double y, double size,
                     HAlign h = HAlign::Left, VAlign v = VAlign::Baseline,
                     TextRotation rot = TextRotation::None, Color color = colors::Black) {
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

// A tiny scatter/line chart built ONLY from the new pieces:
// niceScale -> computeChartLayout -> Viewport -> primitives.
static void drawDemoChart(int width, int height, const std::vector<double>& xs,
                          const std::vector<double>& ys, const std::string& title,
                          bool includeZeroY, const std::string& filename) {
    FontManager& font = FontManager::defaultFont();
    Renderer r(width, height, Color(250, 250, 250));

    double xMin = kInf, xMax = -kInf, yMin = kInf, yMax = -kInf;
    for (double v : xs) if (std::isfinite(v)) { xMin = std::min(xMin, v); xMax = std::max(xMax, v); }
    for (double v : ys) if (std::isfinite(v)) { yMin = std::min(yMin, v); yMax = std::max(yMax, v); }

    const double tickSize = std::max(9.0, std::min(14.0, height / 38.0));
    const double axisSize = tickSize + 2.0;
    const double titleSize = std::max(12.0, std::min(24.0, height / 22.0));

    const AxisScale xScale = niceScale(xMin, xMax, width < 400 ? 4 : 6, false);
    const AxisScale yScale = niceScale(yMin, yMax, 6, includeZeroY);

    double widest = 0.0;
    for (const std::string& s : tickLabels(yScale))
        widest = std::max(widest, measureText(font, s, tickSize).width);

    LayoutOptions opt;
    opt.hasTitle = opt.hasXAxisTitle = opt.hasYAxisTitle = true;
    opt.titleSize = titleSize;
    opt.axisTitleSize = axisSize;
    opt.tickLabelSize = tickSize;
    opt.yTickLabelWidth = widest;
    const ChartLayout L = computeChartLayout(width, height, opt);
    const Viewport vp(L.plotArea, xScale.min, xScale.max, yScale.min, yScale.max);

    Rectangle bg;
    bg.x = L.plotArea.left; bg.y = L.plotArea.top;
    bg.width = L.plotArea.width; bg.height = L.plotArea.height;
    bg.fill = colors::White;
    r.draw(bg);

    // Grid
    const Color gridColor(225, 225, 225);
    for (double t : yScale.ticks) {
        const double py = vp.mapY(t);
        r.draw(Line{L.plotArea.left, py, L.plotArea.right(), py, gridColor, 1});
    }
    for (double t : xScale.ticks) {
        const double px = vp.mapX(t);
        r.draw(Line{px, L.plotArea.top, px, L.plotArea.bottom(), gridColor, 1});
    }
    // Darker lines where x = 0 / y = 0 (only when they are inside the chart)
    if (yScale.min < 0.0 && yScale.max > 0.0) {
        const double py = vp.mapY(0.0);
        r.draw(Line{L.plotArea.left, py, L.plotArea.right(), py, Color(90, 90, 90), 1});
    }
    if (xScale.min < 0.0 && xScale.max > 0.0) {
        const double px = vp.mapX(0.0);
        r.draw(Line{px, L.plotArea.top, px, L.plotArea.bottom(), Color(90, 90, 90), 1});
    }

    // Data, kept inside the plot area (padded 6 px so edge points are not cut in half)
    r.context().setClip(roundToInt(L.plotArea.left) - 6, roundToInt(L.plotArea.top) - 6,
                        roundToInt(L.plotArea.right()) + 6, roundToInt(L.plotArea.bottom()) + 6);
    const std::size_t n = std::min(xs.size(), ys.size());
    for (std::size_t i = 0; i + 1 < n; ++i)
        r.draw(Line{vp.mapX(xs[i]), vp.mapY(ys[i]), vp.mapX(xs[i + 1]), vp.mapY(ys[i + 1]),
                    Color(70, 130, 200), 2});
    for (std::size_t i = 0; i < n; ++i) {
        Point p;
        p.x = vp.mapX(xs[i]);
        p.y = vp.mapY(ys[i]);
        p.color = Color(220, 60, 60);
        p.size = 8;
        r.draw(p);
    }
    r.context().clearClip();

    // Frame
    Rectangle border;
    border.x = L.plotArea.left; border.y = L.plotArea.top;
    border.width = L.plotArea.width; border.height = L.plotArea.height;
    border.stroke = colors::Black;
    r.draw(border);

    // Ticks and labels
    const std::vector<std::string> xLabels = tickLabels(xScale);
    for (std::size_t i = 0; i < xScale.ticks.size(); ++i) {
        const double px = vp.mapX(xScale.ticks[i]);
        r.draw(Line{px, L.plotArea.bottom(), px, L.plotArea.bottom() + L.tickLength, colors::Black, 1});
        r.draw(makeText(xLabels[i], px, L.plotArea.bottom() + L.tickLength + L.tickLabelGap, tickSize,
                        HAlign::Center, VAlign::Top));
    }
    const std::vector<std::string> yLabels = tickLabels(yScale);
    for (std::size_t i = 0; i < yScale.ticks.size(); ++i) {
        const double py = vp.mapY(yScale.ticks[i]);
        r.draw(Line{L.plotArea.left - L.tickLength, py, L.plotArea.left, py, colors::Black, 1});
        r.draw(makeText(yLabels[i], L.plotArea.left - L.tickLength - L.tickLabelGap, py, tickSize,
                        HAlign::Right, VAlign::Middle));
    }

    // Titles
    r.draw(makeText(title, L.titleX, L.titleY, titleSize, HAlign::Center, VAlign::Top));
    r.draw(makeText("X value", L.xTitleX, L.xTitleY, axisSize, HAlign::Center, VAlign::Bottom));
    r.draw(makeText("Y value", L.yTitleX, L.yTitleY, axisSize, HAlign::Center, VAlign::Middle,
                    TextRotation::Up));

    std::cout << (r.savePng(filename) ? "Demo saved: " : "Could not save: ") << filename << "\n";
}

static void drawDemos() {
    std::vector<double> xs, ys;
    for (int i = -6; i <= 6; ++i) {
        xs.push_back(i);
        ys.push_back(40.0 * std::sin(i / 2.0) - 5.0);
    }
    std::cout << "\n";
    drawDemoChart(800, 500, xs, ys, "Negative values (800x500)", false, "output/test_chart_large.png");
    drawDemoChart(320, 240, xs, ys, "Same data (320x240)", false, "output/test_chart_small.png");

    std::vector<double> cx, cy;
    for (int i = 0; i <= 5; ++i) { cx.push_back(i); cy.push_back(7.0); }
    drawDemoChart(600, 400, cx, cy, "Every y value is 7 (zero range)", false, "output/test_chart_constant.png");

    std::vector<double> bx, by;
    const double sales[] = {132, 158, 141, 175, 168, 149};
    for (int i = 0; i < 6; ++i) { bx.push_back(i + 1); by.push_back(sales[i]); }
    drawDemoChart(600, 400, bx, by, "Zero baseline (includeZero = true)", true, "output/test_chart_zero.png");
}

int main() {
    if (!FontManager::defaultFont().isLoaded()) {
        std::cout << "Font not found: check assets/fonts/DejaVuSans.ttf (run from the project folder).\n";
        return 1;
    }
    testViewport();
    testAxis();
    testLayout();
    drawDemos();

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
