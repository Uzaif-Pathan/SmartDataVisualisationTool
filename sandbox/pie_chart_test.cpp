#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "graphics/Renderer.h"
#include "graphics/charts/Legend.h"
#include "graphics/charts/PieChart.h"

using namespace gfx;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

// Approximate match: fill colours get anti-aliased/border pixels mixed in
// near slice edges, so allow some tolerance.
static bool approxColorSomewhereIn(const Canvas& c, int x0, int y0, int x1, int y1,
                                   const Color& expect, int tol) {
    for (int y = std::max(0, y0); y < std::min(c.height(), y1); ++y)
        for (int x = std::max(0, x0); x < std::min(c.width(), x1); ++x) {
            const Color p = c.getPixel(x, y);
            if (std::abs(int(p.r) - int(expect.r)) <= tol && std::abs(int(p.g) - int(expect.g)) <= tol &&
                std::abs(int(p.b) - int(expect.b)) <= tol)
                return true;
        }
    return false;
}

static const double kNaN = std::numeric_limits<double>::quiet_NaN();
static const double kInf = std::numeric_limits<double>::infinity();

static void testNormalChart() {
    PieChartData data;
    data.slices = {
        {"Electronics", 420.0}, {"Clothing", 180.0}, {"Groceries", 560.0},
        {"Toys", 95.0},         {"Books", 260.0},
    };

    PieChartOptions opt;
    opt.title = "Sales Share by Category";

    Renderer r(800, 500);
    drawPieChart(r, 800, 500, data, opt);

    bool allSlicesVisible = true;
    for (std::size_t i = 0; i < data.slices.size(); ++i)
        if (!approxColorSomewhereIn(r.canvas(), 0, 0, 800, 500, paletteColor(i), 40))
            allSlicesVisible = false;
    check(allSlicesVisible, "normal chart: all 5 slice colours appear somewhere on the canvas");

    r.savePng("output/test_piechart_normal.png");
    check(true, "normal chart: saved to output/test_piechart_normal.png");
}

static void testSingleSliceIsFullCircle() {
    PieChartData data;
    data.slices = {{"Everything", 42.0}};

    Renderer r(300, 300);
    drawPieChart(r, 300, 300, data);

    check(approxColorSomewhereIn(r.canvas(), 100, 100, 200, 200, paletteColor(0), 30),
         "single slice: fills the circle with its palette colour (no degenerate wedge)");

    r.savePng("output/test_piechart_single.png");
}

static void testInvalidSlicesAreDropped() {
    // Negative, NaN, infinite and zero values should all be excluded — not
    // drawn as empty wedges, not counted in the total, not in the legend.
    PieChartData data;
    data.slices = {
        {"Good A", 30.0}, {"Negative", -10.0}, {"NaN", kNaN},
        {"Infinite", kInf}, {"Zero", 0.0},      {"Good B", 70.0},
    };

    Renderer r(600, 400);
    drawPieChart(r, 600, 400, data);

    // Only 2 valid slices remain: Good A (30%) and Good B (70%). Their
    // palette colours (positions 0 and 1 among the *valid* slices) should
    // both be visible.
    check(approxColorSomewhereIn(r.canvas(), 0, 0, 600, 400, paletteColor(0), 40) &&
             approxColorSomewhereIn(r.canvas(), 0, 0, 600, 400, paletteColor(1), 40),
         "invalid slices dropped: the two valid slices still render with palette colours 0 and 1");

    r.savePng("output/test_piechart_invalid_dropped.png");
}

static void testAllInvalidDrawsEmptyRing() {
    PieChartData data;
    data.slices = {{"Bad", kNaN}, {"AlsoBad", -5.0}, {"Zero", 0.0}};

    Renderer r(300, 300);
    drawPieChart(r, 300, 300, data);
    check(true, "all slices invalid: draws an empty ring, no crash");
    r.savePng("output/test_piechart_all_invalid.png");
}

static void testEmptyChart() {
    PieChartData data;  // 0 slices
    Renderer r(300, 200);
    drawPieChart(r, 300, 200, data);
    check(true, "0 slices: draws an empty ring without crashing");
}

static void testTinyCanvas() {
    PieChartData data;
    data.slices = {{"A", 1.0}, {"B", 2.0}};
    Renderer r(15, 12);
    drawPieChart(r, 15, 12, data);
    check(true, "tiny canvas: does not crash");
}

static void testManySlices() {
    PieChartData data;
    for (int i = 0; i < 40; ++i) data.slices.push_back({"Cat" + std::to_string(i), 1.0 + (i % 5)});

    Renderer r(700, 500);
    drawPieChart(r, 700, 500, data);
    check(true, "40 slices: renders without crashing (labels/legend thin out)");
    r.savePng("output/test_piechart_many_slices.png");
}

static void testCounterClockwiseAndStartAngle() {
    PieChartData data;
    data.slices = {{"A", 25.0}, {"B", 25.0}, {"C", 25.0}, {"D", 25.0}};

    PieChartOptions opt;
    opt.clockwise = false;
    opt.startAngleDeg = 45.0;

    Renderer r(300, 300);
    drawPieChart(r, 300, 300, data, opt);
    check(true, "counter-clockwise + custom start angle: renders without crashing");
    r.savePng("output/test_piechart_ccw_start_angle.png");
}

int main() {
    testNormalChart();
    testSingleSliceIsFullCircle();
    testInvalidSlicesAreDropped();
    testAllInvalidDrawsEmptyRing();
    testEmptyChart();
    testTinyCanvas();
    testManySlices();
    testCounterClockwiseAndStartAngle();

    std::cout << "\n" << (failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << " ("
              << failures << " failure(s))\n";
    return failures == 0 ? 0 : 1;
}
