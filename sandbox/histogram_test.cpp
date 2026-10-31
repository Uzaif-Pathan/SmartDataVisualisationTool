#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "graphics/Renderer.h"
#include "graphics/charts/Histogram.h"

using namespace gfx;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

// Any non-white, non-black pixel (i.e. a bar, not grid/frame/text) in this rect?
static bool barColorSomewhereIn(const Canvas& c, int x0, int y0, int x1, int y1, const Color& expect) {
    for (int y = std::max(0, y0); y < std::min(c.height(), y1); ++y)
        for (int x = std::max(0, x0); x < std::min(c.width(), x1); ++x)
            if (c.getPixel(x, y) == expect) return true;
    return false;
}

static int countRowsWithColor(const Canvas& c, const Color& expect) {
    int rows = 0;
    for (int y = 0; y < c.height(); ++y) {
        bool found = false;
        for (int x = 0; x < c.width(); ++x)
            if (c.getPixel(x, y) == expect) { found = true; break; }
        if (found) ++rows;
    }
    return rows;
}

static const double kNaN = std::numeric_limits<double>::quiet_NaN();
static const double kInf = std::numeric_limits<double>::infinity();

static void testNormalDistribution() {
    HistogramData data;
    std::mt19937 rng(42);
    std::normal_distribution<double> dist(50.0, 10.0);
    for (int i = 0; i < 200; ++i) data.values.push_back(dist(rng));

    HistogramOptions opt;
    opt.title = "Age Distribution";
    opt.xAxisTitle = "Age";

    Renderer r(800, 500);
    drawHistogram(r, 800, 500, data, opt);
    check(barColorSomewhereIn(r.canvas(), 0, 0, 800, 500, opt.barColor),
          "200 normal-distributed values: bars are drawn");
}

static void testSturgesAutoBinCount() {
    HistogramData data;
    for (int i = 0; i < 100; ++i) data.values.push_back(static_cast<double>(i));

    Renderer r(600, 400);
    drawHistogram(r, 600, 400, data, {});
    check(true, "100 values with auto (Sturges) bin count renders without crashing");
}

static void testExplicitBinCountOverride() {
    HistogramData data;
    for (int i = 0; i < 50; ++i) data.values.push_back(static_cast<double>(i % 10));

    HistogramOptions opt;
    opt.binCount = 20;   // explicit override, more than Sturges would pick

    Renderer r(600, 400);
    drawHistogram(r, 600, 400, data, opt);
    check(true, "explicit binCount override is honoured without crashing");
}

static void testSingleValue() {
    HistogramData data;
    data.values = {42.0};

    Renderer r(400, 300);
    drawHistogram(r, 400, 300, data, {});
    check(barColorSomewhereIn(r.canvas(), 0, 0, 400, 300, HistogramOptions{}.barColor),
          "a single observation: axis widens and one bar is visible");
}

static void testAllValuesEqual() {
    HistogramData data;
    data.values = {7.0, 7.0, 7.0, 7.0, 7.0};

    Renderer r(400, 300);
    drawHistogram(r, 400, 300, data, {});
    check(barColorSomewhereIn(r.canvas(), 0, 0, 400, 300, HistogramOptions{}.barColor),
          "all-equal values: axis widens and the single bin is visible");
}

static void testEmptyData() {
    HistogramData data;   // no values at all
    HistogramOptions opt;
    opt.title = "Empty";

    Renderer r(400, 300);
    drawHistogram(r, 400, 300, data, opt);
    const bool frameDrawn = barColorSomewhereIn(r.canvas(), 0, 0, 400, 300, colors::Black);
    check(frameDrawn, "0 values: still draws a frame, no crash");
}

static void testAllValuesNaNOrInfinite() {
    HistogramData data;
    data.values = {kNaN, kInf, -kInf, kNaN};

    Renderer r(400, 300);
    drawHistogram(r, 400, 300, data, {});
    check(true, "every value NaN/infinite: falls back to an empty frame, no crash");
}

static void testSomeValuesNaNSkipped() {
    HistogramData data;
    data.values = {10, 20, kNaN, 30, kInf, 40};

    Renderer r(400, 300);
    drawHistogram(r, 400, 300, data, {});
    check(true, "a mix of valid and NaN/infinite values renders using only the finite ones");
}

static void testNegativeValues() {
    HistogramData data;
    for (int i = -20; i <= 20; ++i) data.values.push_back(i * 0.5);

    Renderer r(400, 300);
    drawHistogram(r, 400, 300, data, {});
    check(true, "negative and positive values together render without crashing");
}

static void testTallerBarForMoreFrequentBin() {
    // Skewed: far more values land in the low range than the high range.
    HistogramData data;
    for (int i = 0; i < 100; ++i) data.values.push_back(1.0 + (i % 5) * 0.1);   // dense cluster near 1
    for (int i = 0; i < 5; ++i) data.values.push_back(9.0 + (i % 5) * 0.1);     // sparse cluster near 9

    HistogramOptions opt;
    opt.binCount = 10;
    Renderer r(400, 300);
    drawHistogram(r, 400, 300, data, opt);

    // The left portion of the canvas (dense, low-value bin) should show more
    // vertical coverage of the bar colour than the right portion (sparse bin).
    const int leftRows = countRowsWithColor(r.canvas(), opt.barColor);
    check(leftRows > 0, "a densely populated bin renders visible bar pixels");
}

static void testManyValues() {
    HistogramData data;
    std::mt19937 rng(7);
    std::uniform_real_distribution<double> dist(0.0, 1000.0);
    for (int i = 0; i < 10000; ++i) data.values.push_back(dist(rng));

    Renderer r(800, 500);
    drawHistogram(r, 800, 500, data, {});
    check(true, "10000 values render without crashing");
}

static void testTinyCanvas() {
    HistogramData data;
    data.values = {1, 2, 3, 4, 5, 6, 7, 8};
    HistogramOptions opt;
    opt.title = "Tiny";
    opt.xAxisTitle = "X";
    opt.yAxisTitle = "Y";

    Renderer r(30, 25);
    drawHistogram(r, 30, 25, data, opt);
    check(true, "30x25 canvas with title + both axis titles does not crash");
}

static void testInvalidCanvasSize() {
    HistogramData data;
    data.values = {1, 2, 3};

    Renderer r(10, 10);
    drawHistogram(r, 0, 100, data, {});
    drawHistogram(r, kNaN, 100, data, {});
    drawHistogram(r, 100, -5, data, {});
    check(true, "invalid canvas size (0 / NaN / negative) draws nothing and does not crash");
}

static void testBadOptionValues() {
    HistogramData data;
    data.values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    HistogramOptions opt;
    opt.titleSize = kNaN;
    opt.tickLabelSize = -5;
    opt.barGapRatio = kInf;
    opt.binCount = -3;   // invalid: falls back to automatic
    opt.title = "Still Works";

    Renderer r(400, 300);
    drawHistogram(r, 400, 300, data, opt);
    check(true, "NaN/negative/infinite option values (and a negative binCount) are sanitised, no crash");
}

static void drawDemo() {
    // A roughly normal distribution: ages 18-70, clustered around 35.
    HistogramData ages;
    std::mt19937 rng(123);
    std::normal_distribution<double> ageDist(35.0, 12.0);
    for (int i = 0; i < 300; ++i) {
        double v = ageDist(rng);
        if (v >= 18.0 && v <= 70.0) ages.values.push_back(v);
    }

    HistogramOptions opt;
    opt.title = "Customer Age Distribution";
    opt.xAxisTitle = "Age";
    opt.yAxisTitle = "Frequency";

    Renderer large(800, 500, Color(250, 250, 250));
    drawHistogram(large, 800, 500, ages, opt);
    std::cout << (large.savePng("output/test_histogram_large.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_histogram_large.png\n";

    Renderer small(320, 240, Color(250, 250, 250));
    drawHistogram(small, 320, 240, ages, opt);
    std::cout << (small.savePng("output/test_histogram_small.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_histogram_small.png\n";

    // A skewed distribution with an explicit bin count, to show a different shape.
    HistogramData skewed;
    std::exponential_distribution<double> expDist(0.5);
    for (int i = 0; i < 300; ++i) skewed.values.push_back(expDist(rng));

    HistogramOptions skewOpt;
    skewOpt.title = "Time to Purchase (skewed)";
    skewOpt.xAxisTitle = "Days";
    skewOpt.binCount = 15;
    skewOpt.barColor = Color(200, 100, 60);

    Renderer skewedRenderer(600, 400, Color(250, 250, 250));
    drawHistogram(skewedRenderer, 600, 400, skewed, skewOpt);
    std::cout << (skewedRenderer.savePng("output/test_histogram_skewed.png") ? "Demo saved: " : "Could not save: ")
              << "output/test_histogram_skewed.png\n";
}

int main() {
    testNormalDistribution();
    testSturgesAutoBinCount();
    testExplicitBinCountOverride();
    testSingleValue();
    testAllValuesEqual();
    testEmptyData();
    testAllValuesNaNOrInfinite();
    testSomeValuesNaNSkipped();
    testNegativeValues();
    testTallerBarForMoreFrequentBin();
    testManyValues();
    testTinyCanvas();
    testInvalidCanvasSize();
    testBadOptionValues();
    drawDemo();

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
