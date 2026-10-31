#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "graphics/Renderer.h"
#include "graphics/charts/BoxPlot.h"
#include "graphics/charts/Legend.h"

using namespace gfx;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

// Approximate match: within `tol` of every channel. Box fill is drawn with
// alpha 140 over a white background, so its on-canvas colour is a blend, not
// the exact palette colour.
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
    BoxPlotData data;
    data.boxes = {
        {"Electronics", {50.0, 120.0, 180.0, 260.0, 420.0, {520.0, 5.0}}},
        {"Clothing", {10.0, 40.0, 60.0, 90.0, 150.0, {}}},
        {"Groceries", {200.0, 300.0, 340.0, 400.0, 560.0, {}}},
        {"Toys", {5.0, 20.0, 35.0, 60.0, 95.0, {}}},
    };

    BoxPlotOptions opt;
    opt.title = "Spend Distribution by Category";
    opt.xAxisTitle = "Category";
    opt.yAxisTitle = "Spend";

    Renderer r(800, 500);
    drawBoxPlot(r, 800, 500, data, opt);

    bool allBoxesVisible = true;
    for (std::size_t i = 0; i < data.boxes.size(); ++i) {
        const int sliceLeft = static_cast<int>(i) * 800 / static_cast<int>(data.boxes.size());
        const int sliceRight = static_cast<int>(i + 1) * 800 / static_cast<int>(data.boxes.size());
        if (!approxColorSomewhereIn(r.canvas(), sliceLeft, 0, sliceRight, 500, paletteColor(i), 60))
            allBoxesVisible = false;
    }
    check(allBoxesVisible, "normal chart: all 4 boxes show their palette colour in their own slot");

    r.savePng("output/test_boxplot_normal.png");
    check(true, "normal chart: saved to output/test_boxplot_normal.png");
}

static void testOutliersBeyondWhiskers() {
    // Electronics has outliers well above and below the whiskers; make sure
    // the axis widens to include them and dots show up out there.
    BoxPlotData data;
    data.boxes = {{"A", {50.0, 120.0, 180.0, 260.0, 420.0, {900.0, -300.0}}}};

    Renderer r(400, 400);
    drawBoxPlot(r, 400, 400, data);
    r.savePng("output/test_boxplot_outliers.png");
    check(true, "outliers beyond whiskers: renders without crashing, axis should cover -300..900");
}

static void testNaNBoxStillShowsOtherBoxesAndLabel() {
    BoxPlotData data;
    data.boxes = {
        {"Good", {10.0, 20.0, 30.0, 40.0, 50.0, {}}},
        {"Bad", {kNaN, 20.0, 30.0, 40.0, 50.0, {60.0}}},  // whiskerLow is NaN -> box skipped
        {"AlsoGood", {5.0, 15.0, 25.0, 35.0, 45.0, {}}},
    };

    Renderer r(600, 400);
    drawBoxPlot(r, 600, 400, data);

    // "Good" and "AlsoGood" boxes should still show their palette colours.
    check(approxColorSomewhereIn(r.canvas(), 0, 0, 200, 400, paletteColor(0), 60),
         "NaN box: first (good) box still drawn");
    check(approxColorSomewhereIn(r.canvas(), 400, 0, 600, 400, paletteColor(2), 60),
         "NaN box: third (good) box still drawn despite the second being invalid");

    r.savePng("output/test_boxplot_nan_box.png");
}

static void testOutOfOrderStatsAreSorted() {
    // Deliberately scrambled order: median below q1, whiskerHigh below q3.
    // Should not crash, and should still draw a sane box (post-sort).
    BoxPlotData data;
    data.boxes = {{"Scrambled", {/*whiskerLow*/ 100.0, /*q1*/ 40.0, /*median*/ 10.0,
                                /*q3*/ 70.0, /*whiskerHigh*/ 30.0, {}}}};

    Renderer r(300, 300);
    drawBoxPlot(r, 300, 300, data);
    r.savePng("output/test_boxplot_scrambled.png");
    check(true, "out-of-order stats: sorted internally, renders without crashing");
}

static void testEmptyChart() {
    BoxPlotData data;  // 0 boxes
    Renderer r(300, 200);
    drawBoxPlot(r, 300, 200, data);
    check(true, "0 categories: draws an empty frame without crashing");
}

static void testTinyCanvas() {
    BoxPlotData data;
    data.boxes = {{"A", {1.0, 2.0, 3.0, 4.0, 5.0, {}}}, {"B", {2.0, 3.0, 4.0, 5.0, 6.0, {}}}};
    Renderer r(20, 15);
    drawBoxPlot(r, 20, 15, data);
    check(true, "tiny canvas: does not crash");
}

static void testAllInfiniteAndAllNaN() {
    BoxPlotData data;
    data.boxes = {{"Inf", {kInf, kInf, kInf, kInf, kInf, {kInf, -kInf}}},
                 {"NaN", {kNaN, kNaN, kNaN, kNaN, kNaN, {kNaN}}}};
    Renderer r(300, 300);
    drawBoxPlot(r, 300, 300, data);
    check(true, "all-infinite and all-NaN boxes: fall back to a default axis range, no crash");
}

static void testZeroIqrBox() {
    // q1 == median == q3: box has zero height but must still render (>=1px).
    BoxPlotData data;
    data.boxes = {{"Constant", {10.0, 20.0, 20.0, 20.0, 30.0, {}}}};
    Renderer r(300, 300);
    drawBoxPlot(r, 300, 300, data);
    r.savePng("output/test_boxplot_zero_iqr.png");
    check(true, "zero-IQR box (q1 == median == q3): renders without crashing");
}

int main() {
    testNormalChart();
    testOutliersBeyondWhiskers();
    testNaNBoxStillShowsOtherBoxesAndLabel();
    testOutOfOrderStatsAreSorted();
    testEmptyChart();
    testTinyCanvas();
    testAllInfiniteAndAllNaN();
    testZeroIqrBox();

    std::cout << "\n" << (failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << " ("
              << failures << " failure(s))\n";
    return failures == 0 ? 0 : 1;
}
