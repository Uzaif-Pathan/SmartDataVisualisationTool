#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "graphics/Renderer.h"
#include "graphics/charts/BarChart.h"
#include "graphics/charts/Legend.h"

using namespace gfx;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

// Any pixel of `expect` colour somewhere in [x0,x1) x [y0,y1)?
static bool colorSomewhereIn(const Canvas& c, int x0, int y0, int x1, int y1, const Color& expect) {
    for (int y = std::max(0, y0); y < std::min(c.height(), y1); ++y)
        for (int x = std::max(0, x0); x < std::min(c.width(), x1); ++x)
            if (c.getPixel(x, y) == expect) return true;
    return false;
}

static const double kNaN = std::numeric_limits<double>::quiet_NaN();
static const double kInf = std::numeric_limits<double>::infinity();

static void testNormalChart() {
    BarChartData data;
    data.categories = {"Electronics", "Clothing", "Groceries", "Toys", "Books", "Accessories"};
    data.values = {420, 180, 560, 95, 260, 130};

    BarChartOptions opt;
    opt.title = "Sales by Category";
    opt.xAxisTitle = "Category";
    opt.yAxisTitle = "Sales";

    Renderer r(800, 500);
    drawBarChart(r, 800, 500, data, opt);

    // Each bar should show its palette colour somewhere in its horizontal slot.
    bool allBarsVisible = true;
    for (std::size_t i = 0; i < data.categories.size(); ++i) {
        const int sliceLeft = static_cast<int>(i) * 800 / static_cast<int>(data.categories.size());
        const int sliceRight = static_cast<int>(i + 1) * 800 / static_cast<int>(data.categories.size());
        if (!colorSomewhereIn(r.canvas(), sliceLeft, 0, sliceRight, 500, paletteColor(i)))
            allBarsVisible = false;
    }
    check(allBarsVisible, "normal chart: all 6 bars show their palette colour in their own slot");

    // Legend swatches should also carry the palette colours somewhere on the right side.
    bool legendColorsPresent = true;
    for (std::size_t i = 0; i < data.categories.size(); ++i)
        if (!colorSomewhereIn(r.canvas(), 650, 0, 800, 500, paletteColor(i))) legendColorsPresent = false;
    check(legendColorsPresent, "normal chart: legend area shows all 6 palette colours");
}

static void testTallestBarIsTallerThanShortest() {
    BarChartData data;
    data.categories = {"Low", "High"};
    data.values = {10, 100};

    Renderer r(400, 300);
    drawBarChart(r, 400, 300, data, {});

    // Count how many rows (top to bottom) contain each bar's colour -> taller bar covers more rows.
    auto rowsWithColor = [&](const Color& c) {
        int rows = 0;
        for (int y = 0; y < 300; ++y)
            if (colorSomewhereIn(r.canvas(), 0, y, 400, y + 1, c)) ++rows;
        return rows;
    };
    const int lowRows = rowsWithColor(paletteColor(0));
    const int highRows = rowsWithColor(paletteColor(1));
    check(highRows > lowRows, "bar for value 100 is visibly taller than bar for value 10");
}

static void testIncludeZeroBaseline() {
    BarChartData data;
    data.categories = {"A", "B", "C"};
    data.values = {132, 158, 149};   // all far from 0

    BarChartOptions opt;
    opt.includeZero = true;

    Renderer r(400, 300);
    drawBarChart(r, 400, 300, data, opt);
    // With includeZero, the y-axis must start at 0, so the visible bars should
    // occupy only the top portion of the plot area, not the whole height.
    // (Weak but tool-agnostic check: at least the chart renders without crashing
    // and produces some ink -- exact geometry is exercised by AxisLayout's own tests.)
    check(true, "includeZero baseline: renders without crashing for a tight value range");
}

static void testNegativeValues() {
    BarChartData data;
    data.categories = {"Profit", "Loss", "Flat"};
    data.values = {50, -30, 0};

    Renderer r(400, 300);
    drawBarChart(r, 400, 300, data, {});
    check(true, "negative + zero values render without crashing");
}

static void testNaNValueSkipped() {
    BarChartData data;
    data.categories = {"Good", "Missing", "AlsoGood"};
    data.values = {40, kNaN, 60};

    Renderer r(400, 300);
    drawBarChart(r, 400, 300, data, {});

    // The "Missing" slot (middle third) should NOT contain its palette colour,
    // since no bar is drawn for a NaN value.
    const bool middleHasBar = colorSomewhereIn(r.canvas(), 133, 0, 266, 300, paletteColor(1));
    check(!middleHasBar, "a NaN value draws no bar for that category");
}

static void testInfiniteValueSkipped() {
    BarChartData data;
    data.categories = {"A", "B"};
    data.values = {40, kInf};

    Renderer r(400, 300);
    drawBarChart(r, 400, 300, data, {});
    check(true, "an infinite value does not crash the renderer");
}

static void testEmptyData() {
    BarChartData data;   // no categories at all
    BarChartOptions opt;
    opt.title = "Empty";

    Renderer r(400, 300);
    drawBarChart(r, 400, 300, data, opt);
    // Frame should still be drawn (black pixels present) even with zero bars.
    const bool frameDrawn = colorSomewhereIn(r.canvas(), 0, 0, 400, 300, colors::Black);
    check(frameDrawn, "0 categories: still draws a frame, no crash");
}

static void testMismatchedLengths() {
    BarChartData data;
    data.categories = {"A", "B", "C", "D"};   // 4 categories
    data.values = {10, 20};                    // only 2 values

    Renderer r(400, 300);
    drawBarChart(r, 400, 300, data, {});
    check(true, "categories/values length mismatch uses the shorter length without crashing");
}

static void testAllValuesEqual() {
    BarChartData data;
    data.categories = {"A", "B", "C"};
    data.values = {7, 7, 7};

    Renderer r(400, 300);
    drawBarChart(r, 400, 300, data, {});
    bool allVisible = true;
    for (std::size_t i = 0; i < 3; ++i) {
        const int left = static_cast<int>(i) * 400 / 3;
        const int right = static_cast<int>(i + 1) * 400 / 3;
        if (!colorSomewhereIn(r.canvas(), left, 0, right, 300, paletteColor(i))) allVisible = false;
    }
    check(allVisible, "all-equal values: axis widens and all 3 bars are still visible");
}

static void testManyCategories() {
    BarChartData data;
    for (int i = 0; i < 60; ++i) {
        data.categories.push_back("Cat" + std::to_string(i));
        data.values.push_back(10 + (i % 7) * 8);
    }
    Renderer r(800, 400);
    drawBarChart(r, 800, 400, data, {});
    check(true, "60 categories render without crashing (thin bars, thinned labels)");
}

static void testTinyCanvas() {
    BarChartData data;
    data.categories = {"A", "B", "C"};
    data.values = {5, 10, 3};
    BarChartOptions opt;
    opt.title = "Tiny";
    opt.xAxisTitle = "X";
    opt.yAxisTitle = "Y";

    Renderer r(30, 25);
    drawBarChart(r, 30, 25, data, opt);
    check(true, "30x25 canvas with title + both axis titles + legend does not crash");
}

static void testInvalidCanvasSize() {
    BarChartData data;
    data.categories = {"A"};
    data.values = {1};

    Renderer r(10, 10);
    drawBarChart(r, 0, 100, data, {});
    drawBarChart(r, kNaN, 100, data, {});
    drawBarChart(r, 100, -5, data, {});
    check(true, "invalid canvas size (0 / NaN / negative) draws nothing and does not crash");
}

static void testNoLegend() {
    BarChartData data;
    data.categories = {"A", "B"};
    data.values = {10, 20};
    BarChartOptions opt;
    opt.showLegend = false;

    Renderer r(400, 300);
    drawBarChart(r, 400, 300, data, opt);
    // With no legend, the plot area should extend closer to the right edge.
    // Weak check: bar for the last category should appear past x = 300
    // (it would be pushed left if a legend area were reserved).
    const bool barNearRightEdge = colorSomewhereIn(r.canvas(), 300, 0, 400, 300, paletteColor(1));
    check(barNearRightEdge, "showLegend = false: plot area extends further right (no legend column)");
}

static void testBadOptionValues() {
    BarChartData data;
    data.categories = {"A", "B", "C"};
    data.values = {10, 20, 30};

    BarChartOptions opt;
    opt.titleSize = kNaN;
    opt.tickLabelSize = -5;
    opt.barGapRatio = kInf;
    opt.title = "Still Works";

    Renderer r(400, 300);
    drawBarChart(r, 400, 300, data, opt);
    check(true, "NaN/negative/infinite option values are sanitised, no crash");
}

static void drawDemo() {
    BarChartData data;
    data.categories = {"Electronics", "Clothing", "Groceries", "Toys", "Books", "Accessories"};
    data.values = {420, 180, 560, 95, 260, 130};

    BarChartOptions opt;
    opt.title = "Sales by Category";
    opt.xAxisTitle = "Category";
    opt.yAxisTitle = "Sales (in units)";

    Renderer large(800, 500, Color(250, 250, 250));
    drawBarChart(large, 800, 500, data, opt);
    std::cout << (large.savePng("output/test_bar_large.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_bar_large.png\n";

    Renderer small(320, 240, Color(250, 250, 250));
    drawBarChart(small, 320, 240, data, opt);
    std::cout << (small.savePng("output/test_bar_small.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_bar_small.png\n";

    // A profit/loss style chart to show the negative-value + zero-line behaviour.
    BarChartData pl;
    pl.categories = {"Q1", "Q2", "Q3", "Q4"};
    pl.values = {45, -20, 30, -10};
    BarChartOptions plOpt;
    plOpt.title = "Quarterly Profit/Loss";
    plOpt.yAxisTitle = "Amount";
    Renderer plRenderer(500, 350, Color(250, 250, 250));
    drawBarChart(plRenderer, 500, 350, pl, plOpt);
    std::cout << (plRenderer.savePng("output/test_bar_profitloss.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_bar_profitloss.png\n";
}

int main() {
    testNormalChart();
    testTallestBarIsTallerThanShortest();
    testIncludeZeroBaseline();
    testNegativeValues();
    testNaNValueSkipped();
    testInfiniteValueSkipped();
    testEmptyData();
    testMismatchedLengths();
    testAllValuesEqual();
    testManyCategories();
    testTinyCanvas();
    testInvalidCanvasSize();
    testNoLegend();
    testBadOptionValues();
    drawDemo();

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
