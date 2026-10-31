#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "graphics/Canvas.h"
#include "graphics/Color.h"
#include "graphics/GraphicsContext.h"
#include "graphics/Renderer.h"
#include "graphics/primitives/Circle.h"
#include "graphics/primitives/Line.h"
#include "graphics/primitives/Point.h"
#include "graphics/primitives/Polygon.h"
#include "graphics/primitives/Rectangle.h"

using namespace gfx;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

static void testCanvas() {
    Canvas c(10, 8);
    check(c.width() == 10 && c.height() == 8, "canvas size is 10x8");
    check(c.getPixel(0, 0) == colors::White, "new canvas is white");
    c.setPixel(3, 4, colors::Red);
    check(c.getPixel(3, 4) == colors::Red, "setPixel/getPixel round trip");
    c.setPixel(-1, 0, colors::Red);
    c.setPixel(10, 0, colors::Red);
    c.setPixel(0, 8, colors::Red);
    check(true, "out-of-range setPixel does not crash");
    check(c.getPixel(-1, 0) == Color(0, 0, 0, 0), "out-of-range getPixel is transparent");

    bool threw = false;
    try { Canvas bad(0, 10); (void)bad; } catch (const std::invalid_argument&) { threw = true; }
    check(threw, "zero-width canvas throws");

    Canvas b(2, 2);
    b.setPixel(0, 0, Color(255, 0, 0, 128));
    const Color got = b.getPixel(0, 0);
    check(got.r == 255 && got.g == 127 && got.b == 127, "50% red over white blends to (255,127,127)");
}

static void testRectangle() {
    Renderer r(100, 80);
    Rectangle rect;
    rect.x = 10; rect.y = 10; rect.width = 20; rect.height = 10;
    rect.fill = colors::Red;
    r.draw(rect);
    const Canvas& c = r.canvas();
    check(c.getPixel(10, 10) == colors::Red, "rect: top-left pixel filled");
    check(c.getPixel(29, 19) == colors::Red, "rect: bottom-right pixel filled");
    check(c.getPixel(30, 10) == colors::White, "rect: right edge is exclusive");
    check(c.getPixel(9, 10) == colors::White, "rect: pixel left of it is empty");
    check(c.getPixel(10, 20) == colors::White, "rect: bottom edge is exclusive");

    Rectangle boxed;
    boxed.x = 40; boxed.y = 10; boxed.width = 20; boxed.height = 20;
    boxed.fill = colors::Blue;
    boxed.stroke = colors::Black;
    boxed.strokeWidth = 2;
    r.draw(boxed);
    check(c.getPixel(40, 10) == colors::Black, "outline: corner is black");
    check(c.getPixel(41, 11) == colors::Black, "outline: 2 pixels thick");
    check(c.getPixel(42, 12) == colors::Blue, "outline: fill starts inside the border");
    check(c.getPixel(50, 20) == colors::Blue, "outline: centre is filled");
    check(c.getPixel(59, 29) == colors::Black, "outline: opposite corner is black");

    Rectangle thin;
    thin.x = 70; thin.y = 10; thin.width = 0.2; thin.height = 20;
    thin.fill = colors::Green;
    r.draw(thin);
    check(c.getPixel(70, 15) == colors::Green, "very thin rectangle still shows 1 pixel");

    Rectangle negative;
    negative.x = 80; negative.y = 10; negative.width = -5; negative.height = 10;
    negative.fill = colors::Red;
    r.draw(negative);
    check(c.getPixel(78, 12) == colors::White, "negative width draws nothing");
}

static void testLine() {
    Renderer r(100, 80);
    const Canvas& c = r.canvas();
    Line h; h.x0 = 5; h.y0 = 50; h.x1 = 95; h.y1 = 50; h.color = colors::Black;
    r.draw(h);
    check(c.getPixel(5, 50) == colors::Black && c.getPixel(50, 50) == colors::Black &&
          c.getPixel(95, 50) == colors::Black, "horizontal line: both ends and middle");
    check(c.getPixel(50, 51) == colors::White, "horizontal line is 1 pixel thick");

    Line d; d.x0 = 0; d.y0 = 0; d.x1 = 9; d.y1 = 9; d.color = colors::Red;
    r.draw(d);
    bool diagonal = true;
    for (int i = 0; i <= 9; ++i) diagonal = diagonal && c.getPixel(i, i) == colors::Red;
    check(diagonal, "diagonal line covers (0,0)..(9,9) with no gaps");

    Renderer r2(100, 80);
    Line rev; rev.x0 = 9; rev.y0 = 9; rev.x1 = 0; rev.y1 = 0; rev.color = colors::Red;
    r2.draw(rev);
    bool reversed = true;
    for (int i = 0; i <= 9; ++i) reversed = reversed && r2.canvas().getPixel(i, i) == colors::Red;
    check(reversed, "reversed line draws the same pixels");

    Renderer r3(100, 80);
    Line thick; thick.x0 = 5; thick.y0 = 40; thick.x1 = 95; thick.y1 = 40;
    thick.color = colors::Blue; thick.thickness = 3;
    r3.draw(thick);
    check(r3.canvas().getPixel(50, 39) == colors::Blue && r3.canvas().getPixel(50, 41) == colors::Blue,
          "thickness 3: pixels above and below are coloured");
    check(r3.canvas().getPixel(50, 43) == colors::White, "thickness 3: not thicker than that");
}

static void testPointAndCircle() {
    Renderer r(100, 80);
    const Canvas& c = r.canvas();
    Point p1; p1.x = 20; p1.y = 20; p1.color = colors::Red;
    r.draw(p1);
    check(c.getPixel(20, 20) == colors::Red && c.getPixel(21, 20) == colors::White,
          "size-1 point is a single pixel");
    Point p5; p5.x = 40; p5.y = 20; p5.color = colors::Red; p5.size = 5;
    r.draw(p5);
    check(c.getPixel(42, 20) == colors::Red && c.getPixel(43, 20) == colors::White,
          "size-5 point is a disc of diameter 5");

    Renderer r2(100, 80);
    Circle disc; disc.cx = 50; disc.cy = 40; disc.radius = 10; disc.fill = colors::Green;
    r2.draw(disc);
    const Canvas& d = r2.canvas();
    check(d.getPixel(50, 40) == colors::Green, "circle: centre filled");
    check(d.getPixel(60, 40) == colors::Green, "circle: point exactly on the edge filled");
    check(d.getPixel(61, 40) == colors::White, "circle: just outside is empty");
    check(d.getPixel(57, 47) == colors::Green, "circle: diagonal point inside (d^2=98)");
    check(d.getPixel(58, 48) == colors::White, "circle: diagonal point outside (d^2=128)");

    Renderer r3(100, 80);
    Circle ring; ring.cx = 50; ring.cy = 40; ring.radius = 20; ring.stroke = colors::Purple;
    ring.strokeWidth = 2;
    r3.draw(ring);
    const Canvas& g = r3.canvas();
    check(g.getPixel(50, 40) == colors::White, "ring: centre is empty");
    check(g.getPixel(60, 40) == colors::White, "ring: inside the hole is empty");
    check(g.getPixel(70, 40) == colors::Purple, "ring: pixel on the ring is coloured");
    check(g.getPixel(50, 20) == colors::Purple, "ring: top of the ring is coloured");
}

static void testPolygon() {
    Renderer r(100, 80);
    const Canvas& c = r.canvas();
    Polygon tri;
    tri.vertices = {{10, 10}, {50, 10}, {10, 50}};
    tri.fill = colors::Orange;
    r.draw(tri);
    check(c.getPixel(15, 15) == colors::Orange, "triangle: inside pixel filled");
    check(c.getPixel(44, 15) == colors::Orange, "triangle: near the slanted edge filled");
    check(c.getPixel(45, 15) == colors::White, "triangle: past the slanted edge empty");
    check(c.getPixel(45, 45) == colors::White, "triangle: far corner empty");

    Renderer r2(100, 80);
    Polygon outline;
    outline.vertices = {{10, 10}, {50, 10}, {50, 50}, {10, 50}};
    outline.stroke = colors::Black;
    r2.draw(outline);
    check(r2.canvas().getPixel(30, 10) == colors::Black && r2.canvas().getPixel(50, 30) == colors::Black,
          "polygon outline: edges drawn");
    check(r2.canvas().getPixel(30, 30) == colors::White, "polygon outline: inside not filled");
}

static void testClipping() {
    Renderer r(100, 80);
    r.context().setClip(0, 0, 50, 80);
    Rectangle wide; wide.x = 0; wide.y = 10; wide.width = 100; wide.height = 10;
    wide.fill = colors::Red;
    r.draw(wide);
    check(r.canvas().getPixel(49, 10) == colors::Red, "clip: inside the clip area is drawn");
    check(r.canvas().getPixel(50, 10) == colors::White, "clip: outside the clip area is not");
    check(r.canvas().getPixel(60, 15) == colors::White, "clip: far outside is not");
    r.context().clearClip();
    r.draw(wide);
    check(r.canvas().getPixel(60, 15) == colors::Red, "clearClip: drawing works everywhere again");
}

static void testExtremeInput() {
    Renderer r(100, 80);
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();

    Line huge; huge.x0 = -1e9; huge.y0 = -1e9; huge.x1 = 1e9; huge.y1 = 1e9; huge.color = colors::Black;
    r.draw(huge);
    check(true, "huge line finishes without hanging");

    Circle giant; giant.cx = 50; giant.cy = 40; giant.radius = 1e9; giant.fill = colors::Green;
    r.draw(giant);
    check(r.canvas().getPixel(0, 0) == colors::Green, "giant circle covers the canvas quickly");

    Renderer r2(100, 80);
    Rectangle bad; bad.x = nan; bad.y = 0; bad.width = 10; bad.height = 10; bad.fill = colors::Red;
    r2.draw(bad);
    Line badLine; badLine.x0 = 0; badLine.y0 = 0; badLine.x1 = inf; badLine.y1 = 10; badLine.color = colors::Red;
    r2.draw(badLine);
    Polygon badPoly; badPoly.vertices = {{0, 0}, {10, nan}, {5, 10}}; badPoly.fill = colors::Red;
    r2.draw(badPoly);
    Circle badCircle; badCircle.cx = 50; badCircle.cy = 40; badCircle.radius = -5; badCircle.fill = colors::Red;
    r2.draw(badCircle);
    bool clean = true;
    for (int y = 0; y < 80; ++y)
        for (int x = 0; x < 100; ++x) clean = clean && r2.canvas().getPixel(x, y) == colors::White;
    check(clean, "NaN / infinite / negative-radius shapes draw nothing");
}

static void testPng() {
    Renderer r(50, 40);
    r.clear(colors::Blue);
    check(r.savePng("output/test_png_check.png"), "savePng succeeds");
    std::ifstream f("output/test_png_check.png", std::ios::binary);
    unsigned char sig[4] = {0, 0, 0, 0};
    f.read(reinterpret_cast<char*>(sig), 4);
    check(f && sig[0] == 0x89 && sig[1] == 'P' && sig[2] == 'N' && sig[3] == 'G',
          "file starts with the PNG signature");
    check(!r.savePng("/no_such_folder_xyz/out.png"), "savePng returns false for a bad path");
}

// A picture to look at with your own eyes.
static void drawDemo() {
    Renderer r(800, 600, Color(250, 250, 250));

    // Light grid
    for (int x = 0; x <= 800; x += 50) {
        Line l; l.x0 = x; l.y0 = 0; l.x1 = x; l.y1 = 600; l.color = Color(228, 228, 228);
        r.draw(l);
    }
    for (int y = 0; y <= 600; y += 50) {
        Line l; l.x0 = 0; l.y0 = y; l.x1 = 800; l.y1 = y; l.color = Color(228, 228, 228);
        r.draw(l);
    }

    // Bars + axis
    for (int i = 0; i < 5; ++i) {
        const double h = 40 + i * 35;
        Rectangle bar;
        bar.x = 40 + i * 60; bar.y = 300 - h; bar.width = 45; bar.height = h;
        bar.fill = Color(70, 130, 200);
        bar.stroke = Color(30, 60, 110);
        bar.strokeWidth = 2;
        r.draw(bar);
    }
    Line axis; axis.x0 = 30; axis.y0 = 300; axis.x1 = 360; axis.y1 = 300;
    axis.color = colors::Black; axis.thickness = 2;
    r.draw(axis);

    // Circles growing in size
    for (int i = 0; i < 6; ++i) {
        Circle c;
        c.cx = 60 + i * 55; c.cy = 100; c.radius = 8 + i * 4;
        c.fill = colors::Orange; c.stroke = Color(120, 60, 0); c.strokeWidth = 2;
        r.draw(c);
    }

    // Fan of lines (thin and thick alternate)
    for (int i = 0; i < 24; ++i) {
        const double a = i * 15.0 * 3.14159265358979 / 180.0;
        Line l;
        l.x0 = 600; l.y0 = 170; l.x1 = 600 + 120 * std::cos(a); l.y1 = 170 + 120 * std::sin(a);
        l.color = Color(static_cast<std::uint8_t>(i * 10), 100, 200);
        l.thickness = (i % 2 == 0) ? 1 : 3;
        r.draw(l);
    }

    // Five-pointed star
    Polygon star;
    for (int i = 0; i < 10; ++i) {
        const double a = -3.14159265358979 / 2 + i * 3.14159265358979 / 5;
        const double rad = (i % 2 == 0) ? 90 : 40;
        star.vertices.push_back({150 + rad * std::cos(a), 450 + rad * std::sin(a)});
    }
    star.fill = Color(240, 200, 40);
    star.stroke = Color(150, 110, 0);
    star.strokeWidth = 2;
    r.draw(star);

    // Donut-style ring
    Circle donut;
    donut.cx = 380; donut.cy = 450; donut.radius = 60;
    donut.stroke = colors::Purple; donut.strokeWidth = 24;
    r.draw(donut);

    // Two translucent shapes overlapping (checks blending)
    Polygon tri;
    tri.vertices = {{550, 380}, {700, 380}, {625, 520}};
    tri.fill = Color(220, 50, 50, 140);
    r.draw(tri);
    Rectangle over;
    over.x = 560; over.y = 400; over.width = 100; over.height = 80;
    over.fill = Color(50, 50, 220, 140);
    r.draw(over);

    // Points of growing size
    for (int i = 1; i <= 9; ++i) {
        Point p; p.x = 40 + i * 30; p.y = 570; p.color = colors::Black; p.size = i;
        r.draw(p);
    }

    // Clipping demo: a huge circle cut to a small box
    r.context().setClip(700, 20, 780, 100);
    Circle big; big.cx = 740; big.cy = 60; big.radius = 80; big.fill = colors::Green;
    r.draw(big);
    r.context().clearClip();
    Rectangle box; box.x = 700; box.y = 20; box.width = 80; box.height = 80;
    box.stroke = colors::Black; box.strokeWidth = 1;
    r.draw(box);

    std::cout << (r.savePng("output/test_shapes.png")
                      ? "\nDemo saved: output/test_shapes.png\n"
                      : "\nCould not save output/test_shapes.png (does the output folder exist?)\n");
}

int main() {
    testCanvas();
    testRectangle();
    testLine();
    testPointAndCircle();
    testPolygon();
    testClipping();
    testExtremeInput();
    testPng();
    drawDemo();

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
