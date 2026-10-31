#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "graphics/Renderer.h"
#include "graphics/charts/LineChart.h"
#include "graphics/charts/Legend.h"

using namespace gfx;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
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
    LineChartData data;
    LineSeries s;
    s.name = "Sales";
    s.x = {1, 2, 3, 4, 5};
    s.y = {10, 25, 15, 40, 30};
    data.series = {s};

    LineChartOptions opt;
    opt.title = "Monthly Sales";
    opt.xAxisTitle = "Month";
    opt.yAxisTitle = "Sales";

    Renderer r(800, 500);
    drawLineChart(r, 800, 500, data, opt);
    check(colorSomewhereIn(r.canvas(), 0, 0, 800, 500, paletteColor(0)),
          "single series: line colour appears somewhere in the plot");
}

static void testMultipleSeries() {
    LineChartData data;
    LineSeries a;
    a.name = "Product A";
    a.x = {1, 2, 3, 4};
    a.y = {10, 20, 15, 25};
    LineSeries b;
    b.name = "Product B";
    b.x = {1, 2, 3, 4};
    b.y = {5, 8, 22, 18};
    data.series = {a, b};

    Renderer r(600, 400);
    drawLineChart(r, 600, 400, data, {});
    check(colorSomewhereIn(r.canvas(), 0, 0, 600, 400, paletteColor(0)) &&
          colorSomewhereIn(r.canvas(), 0, 0, 600, 400, paletteColor(1)),
          "two series: both palette colours appear");
}

static void testDifferentRangesCombinedAxis() {
    LineChartData data;
    LineSeries small;
    small.name = "Small";
    small.x = {0, 1, 2};
    small.y = {1, 2, 1.5};
    LineSeries big;
    big.name = "Big";
    big.x = {0, 1, 2};
    big.y = {1000, 2000, 1500};
    data.series = {small, big};

    Renderer r(600, 400);
    drawLineChart(r, 600, 400, data, {});
    check(true, "two series with very different y ranges render without crashing");
}

static void testSinglePointSeries() {
    LineChartData data;
    LineSeries s;
    s.name = "Lonely";
    s.x = {5};
    s.y = {5};
    data.series = {s};

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, {});
    check(colorSomewhereIn(r.canvas(), 0, 0, 400, 300, paletteColor(0)),
          "a single-point series draws just its marker, no crash");
}

static void testEmptySeries() {
    LineChartData data;
    LineSeries s;
    s.name = "Empty";
    data.series = {s};   // no points at all

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, {});
    const bool frameDrawn = colorSomewhereIn(r.canvas(), 0, 0, 400, 300, colors::Black);
    check(frameDrawn, "a series with 0 points: still draws a frame, no crash");
}

static void testNoSeriesAtAll() {
    LineChartData data;   // data.series is empty
    LineChartOptions opt;
    opt.title = "Empty Chart";

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, opt);
    const bool frameDrawn = colorSomewhereIn(r.canvas(), 0, 0, 400, 300, colors::Black);
    check(frameDrawn, "0 series: still draws a frame, no crash");
}

static void testNaNBreaksLineButKeepsRest() {
    LineChartData data;
    LineSeries s;
    s.name = "Gapped";
    s.x = {1, 2, 3, 4, 5};
    s.y = {10, kNaN, 30, 40, 50};
    data.series = {s};

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, {});
    check(true, "a NaN point breaks the line there but the rest still renders");
}

static void testInfiniteValueSkipped() {
    LineChartData data;
    LineSeries s;
    s.name = "Inf";
    s.x = {1, 2, 3};
    s.y = {10, kInf, 30};
    data.series = {s};

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, {});
    check(true, "an infinite value does not crash the renderer");
}

static void testMismatchedXYLength() {
    LineChartData data;
    LineSeries s;
    s.name = "Mismatched";
    s.x = {1, 2, 3, 4, 5};   // 5 x values
    s.y = {10, 20};          // only 2 y values

    data.series = {s};
    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, {});
    check(true, "x/y length mismatch uses the shorter length without crashing");
}

static void testUnsortedX() {
    LineChartData data;
    LineSeries s;
    s.name = "Unsorted";
    s.x = {5, 1, 3, 2, 4};   // deliberately out of order
    s.y = {50, 10, 30, 20, 40};
    data.series = {s};

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, {});
    check(true, "unsorted x values connect in the given order without crashing (zigzag expected)");
}

static void testAllPointsIdentical() {
    LineChartData data;
    LineSeries s;
    s.name = "Flat";
    s.x = {1, 1, 1, 1};
    s.y = {7, 7, 7, 7};
    data.series = {s};

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, {});
    check(colorSomewhereIn(r.canvas(), 0, 0, 400, 300, paletteColor(0)),
          "every point identical: both axes widen and the point cluster is visible");
}

static void testIncludeZeroY() {
    LineChartData data;
    LineSeries s;
    s.x = {1, 2, 3};
    s.y = {132, 158, 149};   // all far from 0
    data.series = {s};

    LineChartOptions opt;
    opt.includeZeroY = true;

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, opt);
    check(true, "includeZeroY: renders without crashing for a tight value range far from 0");
}

static void testManyPoints() {
    LineChartData data;
    LineSeries s;
    s.name = "Dense";
    for (int i = 0; i < 5000; ++i) {
        s.x.push_back(i);
        s.y.push_back(std::sin(i * 0.01) * 100.0);
    }
    data.series = {s};

    Renderer r(800, 500);
    drawLineChart(r, 800, 500, data, {});
    check(true, "5000 points render without crashing");
}

static void testNoMarkers() {
    LineChartData data;
    LineSeries s;
    s.x = {1, 2, 3};
    s.y = {10, 20, 15};
    data.series = {s};

    LineChartOptions opt;
    opt.showMarkers = false;

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, opt);
    check(true, "showMarkers = false renders without crashing");
}

static void testNoLegendWhenUnnamed() {
    LineChartData data;
    LineSeries s;   // no name given
    s.x = {1, 2, 3};
    s.y = {10, 20, 15};
    data.series = {s};

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, {});
    // With no named series, the plot area should extend closer to the right edge
    // (no legend column reserved).
    const bool lineNearRightEdge = colorSomewhereIn(r.canvas(), 300, 0, 400, 300, paletteColor(0));
    check(lineNearRightEdge, "unnamed series: no legend reserved, plot area extends further right");
}

static void testTinyCanvas() {
    LineChartData data;
    LineSeries s;
    s.name = "Tiny";
    s.x = {1, 2, 3};
    s.y = {5, 10, 3};
    data.series = {s};

    LineChartOptions opt;
    opt.title = "Tiny";
    opt.xAxisTitle = "X";
    opt.yAxisTitle = "Y";

    Renderer r(30, 25);
    drawLineChart(r, 30, 25, data, opt);
    check(true, "30x25 canvas with title + both axis titles + legend does not crash");
}

static void testInvalidCanvasSize() {
    LineChartData data;
    LineSeries s;
    s.x = {1, 2};
    s.y = {1, 2};
    data.series = {s};

    Renderer r(10, 10);
    drawLineChart(r, 0, 100, data, {});
    drawLineChart(r, kNaN, 100, data, {});
    drawLineChart(r, 100, -5, data, {});
    check(true, "invalid canvas size (0 / NaN / negative) draws nothing and does not crash");
}

static void testBadOptionValues() {
    LineChartData data;
    LineSeries s;
    s.x = {1, 2, 3};
    s.y = {10, 20, 15};
    data.series = {s};

    LineChartOptions opt;
    opt.titleSize = kNaN;
    opt.tickLabelSize = -5;
    opt.lineWidth = kInf;
    opt.markerRadius = -3;
    opt.title = "Still Works";

    Renderer r(400, 300);
    drawLineChart(r, 400, 300, data, opt);
    check(true, "NaN/negative/infinite option values are sanitised, no crash");
}

static void drawDemo() {
    LineChartData data;
    LineSeries a;
    a.name = "2025";
    a.x = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    a.y = {120, 135, 128, 150, 165, 180, 175, 190, 185, 200, 210, 225};
    LineSeries b;
    b.name = "2026";
    b.x = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    b.y = {130, 128, 140, 155, 170, 175, 190, 195, 188, 210, 205, 230};
    data.series = {a, b};

    LineChartOptions opt;
    opt.title = "Monthly Revenue: 2025 vs 2026";
    opt.xAxisTitle = "Month";
    opt.yAxisTitle = "Revenue (in thousands)";

    Renderer large(800, 500, Color(250, 250, 250));
    drawLineChart(large, 800, 500, data, opt);
    std::cout << (large.savePng("output/test_line_large.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_line_large.png\n";

    Renderer small(320, 240, Color(250, 250, 250));
    drawLineChart(small, 320, 240, data, opt);
    std::cout << (small.savePng("output/test_line_small.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_line_small.png\n";

    // A single noisy series with a NaN gap, to show the break-in-the-line behaviour.
    LineChartData noisy;
    LineSeries n;
    n.name = "Sensor Reading";
    for (int i = 0; i <= 30; ++i) {
        n.x.push_back(i);
        if (i >= 12 && i <= 15) {
            n.y.push_back(kNaN);   // a missing-data gap
        } else {
            n.y.push_back(50.0 + 20.0 * std::sin(i * 0.3) + (i % 3) * 2.0);
        }
    }
    noisy.series = {n};
    LineChartOptions noisyOpt;
    noisyOpt.title = "Sensor Reading Over Time (with a data gap)";
    noisyOpt.xAxisTitle = "Time (minutes)";
    noisyOpt.yAxisTitle = "Reading";
    noisyOpt.showLegend = false;

    Renderer gapRenderer(700, 400, Color(250, 250, 250));
    drawLineChart(gapRenderer, 700, 400, noisy, noisyOpt);
    std::cout << (gapRenderer.savePng("output/test_line_gap.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_line_gap.png\n";
}

int main() {
    testSingleSeries();
    testMultipleSeries();
    testDifferentRangesCombinedAxis();
    testSinglePointSeries();
    testEmptySeries();
    testNoSeriesAtAll();
    testNaNBreaksLineButKeepsRest();
    testInfiniteValueSkipped();
    testMismatchedXYLength();
    testUnsortedX();
    testAllPointsIdentical();
    testIncludeZeroY();
    testManyPoints();
    testNoMarkers();
    testNoLegendWhenUnnamed();
    testTinyCanvas();
    testInvalidCanvasSize();
    testBadOptionValues();
    drawDemo();

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
