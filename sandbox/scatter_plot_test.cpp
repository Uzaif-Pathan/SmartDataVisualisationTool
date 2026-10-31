#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "graphics/Renderer.h"
#include "graphics/charts/Legend.h"
#include "graphics/charts/ScatterPlot.h"

using namespace gfx;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

// Looser than exact-colour match, since points are drawn translucent by
// default (blended with the white background) rather than at full opacity.
static bool nonBackgroundSomewhereIn(const Canvas& c, int x0, int y0, int x1, int y1,
                                     const Color& background) {
    for (int y = std::max(0, y0); y < std::min(c.height(), y1); ++y)
        for (int x = std::max(0, x0); x < std::min(c.width(), x1); ++x)
            if (c.getPixel(x, y) != background) return true;
    return false;
}

static bool colorSomewhereIn(const Canvas& c, int x0, int y0, int x1, int y1, const Color& expect) {
    for (int y = std::max(0, y0); y < std::min(c.height(), y1); ++y)
        for (int x = std::max(0, x0); x < std::min(c.width(), x1); ++x)
            if (c.getPixel(x, y) == expect) return true;
    return false;
}

static const double kNaN = std::numeric_limits<double>::quiet_NaN();
static const double kInf = std::numeric_limits<double>::infinity();

static void testSingleSeries() {
    ScatterPlotData data;
    ScatterSeries s;
    s.name = "Observations";
    s.x = {1, 2, 3, 4, 5, 2.5, 3.5};
    s.y = {10, 15, 12, 20, 18, 14, 16};
    data.series = {s};

    ScatterPlotOptions opt;
    opt.title = "Height vs Weight";
    opt.xAxisTitle = "Height";
    opt.yAxisTitle = "Weight";

    Renderer r(800, 500);
    drawScatterPlot(r, 800, 500, data, opt);
    check(nonBackgroundSomewhereIn(r.canvas(), 0, 0, 800, 500, colors::White),
          "single series: points are drawn somewhere in the plot");
}

static void testMultipleSeries() {
    ScatterPlotData data;
    ScatterSeries a;
    a.name = "Group A";
    a.x = {1, 2, 3};
    a.y = {10, 12, 11};
    ScatterSeries b;
    b.name = "Group B";
    b.x = {4, 5, 6};
    b.y = {20, 22, 21};
    data.series = {a, b};

    ScatterPlotOptions opt;
    opt.pointOpacity = 1.0;   // full opacity so exact palette colours can be checked

    Renderer r(600, 400);
    drawScatterPlot(r, 600, 400, data, opt);
    check(colorSomewhereIn(r.canvas(), 0, 0, 600, 400, paletteColor(0)) &&
          colorSomewhereIn(r.canvas(), 0, 0, 600, 400, paletteColor(1)),
          "two series: both palette colours appear at full opacity");
}

static void testEmptySeries() {
    ScatterPlotData data;
    ScatterSeries s;
    s.name = "Empty";
    data.series = {s};   // no points

    Renderer r(400, 300);
    drawScatterPlot(r, 400, 300, data, {});
    const bool frameDrawn = colorSomewhereIn(r.canvas(), 0, 0, 400, 300, colors::Black);
    check(frameDrawn, "a series with 0 points: still draws a frame, no crash");
}

static void testNoSeriesAtAll() {
    ScatterPlotData data;   // no series
    ScatterPlotOptions opt;
    opt.title = "Empty Chart";

    Renderer r(400, 300);
    drawScatterPlot(r, 400, 300, data, opt);
    const bool frameDrawn = colorSomewhereIn(r.canvas(), 0, 0, 400, 300, colors::Black);
    check(frameDrawn, "0 series: still draws a frame, no crash");
}

static void testNaNPointSkipped() {
    ScatterPlotData data;
    ScatterSeries s;
    s.x = {1, kNaN, 3};
    s.y = {10, 20, 30};
    data.series = {s};

    Renderer r(400, 300);
    drawScatterPlot(r, 400, 300, data, {});
    check(true, "a NaN point is skipped; the other points still render, no crash");
}

static void testInfinitePointSkipped() {
    ScatterPlotData data;
    ScatterSeries s;
    s.x = {1, 2, 3};
    s.y = {10, kInf, 30};
    data.series = {s};

    Renderer r(400, 300);
    drawScatterPlot(r, 400, 300, data, {});
    check(true, "an infinite point is skipped, no crash");
}

static void testMismatchedXYLength() {
    ScatterPlotData data;
    ScatterSeries s;
    s.x = {1, 2, 3, 4, 5};
    s.y = {10, 20};

    data.series = {s};
    Renderer r(400, 300);
    drawScatterPlot(r, 400, 300, data, {});
    check(true, "x/y length mismatch uses the shorter length without crashing");
}

static void testAllPointsIdentical() {
    ScatterPlotData data;
    ScatterSeries s;
    s.x = {7, 7, 7, 7};
    s.y = {3, 3, 3, 3};
    data.series = {s};

    ScatterPlotOptions opt;
    opt.pointOpacity = 1.0;

    Renderer r(400, 300);
    drawScatterPlot(r, 400, 300, data, opt);
    check(colorSomewhereIn(r.canvas(), 0, 0, 400, 300, paletteColor(0)),
          "every point identical: both axes widen and the point cluster is visible");
}

static void testIncludeZero() {
    ScatterPlotData data;
    ScatterSeries s;
    s.x = {100, 120, 110};
    s.y = {500, 520, 510};
    data.series = {s};

    ScatterPlotOptions opt;
    opt.includeZeroX = true;
    opt.includeZeroY = true;

    Renderer r(400, 300);
    drawScatterPlot(r, 400, 300, data, opt);
    check(true, "includeZeroX/Y: renders without crashing for values far from 0");
}

static void testManyPoints() {
    ScatterPlotData data;
    ScatterSeries s;
    std::mt19937 rng(3);
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    for (int i = 0; i < 5000; ++i) {
        s.x.push_back(dist(rng));
        s.y.push_back(dist(rng));
    }
    data.series = {s};

    Renderer r(800, 500);
    drawScatterPlot(r, 800, 500, data, {});
    check(true, "5000 points render without crashing");
}

static void testNoLegendWhenUnnamed() {
    ScatterPlotData data;
    ScatterSeries s;   // no name
    s.x = {1, 2, 3};
    s.y = {10, 20, 15};
    data.series = {s};

    ScatterPlotOptions opt;
    opt.pointOpacity = 1.0;

    Renderer r(400, 300);
    drawScatterPlot(r, 400, 300, data, opt);
    const bool pointNearRightEdge = colorSomewhereIn(r.canvas(), 300, 0, 400, 300, paletteColor(0));
    check(pointNearRightEdge, "unnamed series: no legend reserved, plot area extends further right");
}

static void testTinyCanvas() {
    ScatterPlotData data;
    ScatterSeries s;
    s.name = "Tiny";
    s.x = {1, 2, 3};
    s.y = {5, 10, 3};
    data.series = {s};

    ScatterPlotOptions opt;
    opt.title = "Tiny";
    opt.xAxisTitle = "X";
    opt.yAxisTitle = "Y";

    Renderer r(30, 25);
    drawScatterPlot(r, 30, 25, data, opt);
    check(true, "30x25 canvas with title + both axis titles + legend does not crash");
}

static void testInvalidCanvasSize() {
    ScatterPlotData data;
    ScatterSeries s;
    s.x = {1, 2};
    s.y = {1, 2};
    data.series = {s};

    Renderer r(10, 10);
    drawScatterPlot(r, 0, 100, data, {});
    drawScatterPlot(r, kNaN, 100, data, {});
    drawScatterPlot(r, 100, -5, data, {});
    check(true, "invalid canvas size (0 / NaN / negative) draws nothing and does not crash");
}

static void testBadOptionValues() {
    ScatterPlotData data;
    ScatterSeries s;
    s.x = {1, 2, 3};
    s.y = {10, 20, 15};
    data.series = {s};

    ScatterPlotOptions opt;
    opt.titleSize = kNaN;
    opt.tickLabelSize = -5;
    opt.pointRadius = kInf;
    opt.pointOpacity = -3.0;
    opt.title = "Still Works";

    Renderer r(400, 300);
    drawScatterPlot(r, 400, 300, data, opt);
    check(true, "NaN/negative/infinite option values are sanitised, no crash");
}

static void drawDemo() {
    // Positive correlation between two variables, with some scatter.
    ScatterPlotData data;
    ScatterSeries s;
    s.name = "Products";
    std::mt19937 rng(11);
    std::normal_distribution<double> noise(0.0, 8.0);
    for (int i = 0; i < 150; ++i) {
        const double price = 10.0 + i * 0.6;
        s.x.push_back(price);
        s.y.push_back(price * 1.4 + noise(rng));
    }
    data.series = {s};

    ScatterPlotOptions opt;
    opt.title = "Price vs Units Sold";
    opt.xAxisTitle = "Price";
    opt.yAxisTitle = "Units Sold";

    Renderer large(800, 500, Color(250, 250, 250));
    drawScatterPlot(large, 800, 500, data, opt);
    std::cout << (large.savePng("output/test_scatter_large.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_scatter_large.png\n";

    Renderer small(320, 240, Color(250, 250, 250));
    drawScatterPlot(small, 320, 240, data, opt);
    std::cout << (small.savePng("output/test_scatter_small.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_scatter_small.png\n";

    // Two overlapping groups, to show legend + colour separation + translucency.
    ScatterPlotData groups;
    ScatterSeries groupA, groupB;
    groupA.name = "Region North";
    groupB.name = "Region South";
    std::normal_distribution<double> ax(30.0, 8.0), ay(60.0, 10.0);
    std::normal_distribution<double> bx(45.0, 8.0), by(50.0, 10.0);
    for (int i = 0; i < 120; ++i) {
        groupA.x.push_back(ax(rng));
        groupA.y.push_back(ay(rng));
        groupB.x.push_back(bx(rng));
        groupB.y.push_back(by(rng));
    }
    groups.series = {groupA, groupB};

    ScatterPlotOptions groupOpt;
    groupOpt.title = "Customer Spend by Region";
    groupOpt.xAxisTitle = "Age";
    groupOpt.yAxisTitle = "Monthly Spend";

    Renderer groupRenderer(700, 450, Color(250, 250, 250));
    drawScatterPlot(groupRenderer, 700, 450, groups, groupOpt);
    std::cout << (groupRenderer.savePng("output/test_scatter_groups.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_scatter_groups.png\n";
}

int main() {
    testSingleSeries();
    testMultipleSeries();
    testEmptySeries();
    testNoSeriesAtAll();
    testNaNPointSkipped();
    testInfinitePointSkipped();
    testMismatchedXYLength();
    testAllPointsIdentical();
    testIncludeZero();
    testManyPoints();
    testNoLegendWhenUnnamed();
    testTinyCanvas();
    testInvalidCanvasSize();
    testBadOptionValues();
    drawDemo();

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
