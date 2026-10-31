#pragma once
#include <optional>
#include <vector>

#include "graphics/Color.h"
#include "graphics/GraphicsContext.h"

namespace gfx {

struct Vertex {
    double x = 0.0;
    double y = 0.0;
};

// A closed shape made of straight edges.  The fill uses the scanline
// "even-odd" method: on every pixel row, find where the edges cross the row,
// sort the crossings, and fill between pairs.  Pie-chart slices, areas and
// arrows are all polygons.
// Needs >= 3 vertices to fill, >= 2 to outline.  Any NaN/infinite vertex
// makes the whole polygon draw nothing.
struct Polygon {
    std::vector<Vertex> vertices;
    std::optional<Color> fill;
    std::optional<Color> stroke;
    int strokeWidth = 1;

    void draw(GraphicsContext& ctx) const;
};

}  // namespace gfx
