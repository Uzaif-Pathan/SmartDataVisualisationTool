#pragma once
#include "graphics/Color.h"
#include "graphics/GraphicsContext.h"

namespace gfx {

// A straight line from (x0,y0) to (x1,y1), drawn with Bresenham's algorithm
// (integer-only, no gaps, works for every slope and direction).
// thickness > 1 stamps a filled circle at every step.
// Use OPAQUE colors for lines: the stamps overlap, so translucent thick
// lines would look uneven.
struct Line {
    double x0 = 0.0;
    double y0 = 0.0;
    double x1 = 0.0;
    double y1 = 0.0;
    Color color;
    int thickness = 1;

    void draw(GraphicsContext& ctx) const;
};

}  // namespace gfx
