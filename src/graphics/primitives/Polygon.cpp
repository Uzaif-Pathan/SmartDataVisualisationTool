#include "graphics/primitives/Polygon.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "graphics/primitives/Line.h"

namespace gfx {
namespace {

void fillPolygon(GraphicsContext& ctx, const std::vector<Vertex>& v, const Color& color) {
    double minY = v[0].y;
    double maxY = v[0].y;
    for (const Vertex& p : v) {
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    const int yStart = std::max(ceilToInt(minY), ctx.clipY0());
    const int yEnd = std::min(floorToInt(maxY), ctx.clipY1() - 1);

    std::vector<double> crossings;
    for (int y = yStart; y <= yEnd; ++y) {
        crossings.clear();
        for (std::size_t i = 0; i < v.size(); ++i) {
            const Vertex& a = v[i];
            const Vertex& b = v[(i + 1) % v.size()];
            // Half-open rule: an edge counts if the row is at its top end but
            // not its bottom end.  This avoids double-counting shared corners.
            if ((a.y <= y && y < b.y) || (b.y <= y && y < a.y)) {
                const double t = (y - a.y) / (b.y - a.y);
                crossings.push_back(a.x + t * (b.x - a.x));
            }
        }
        std::sort(crossings.begin(), crossings.end());
        for (std::size_t k = 0; k + 1 < crossings.size(); k += 2)
            ctx.hspan(ceilToInt(crossings[k]), ceilToInt(crossings[k + 1]), y, color);
    }
}

}  // namespace

void Polygon::draw(GraphicsContext& ctx) const {
    if (vertices.size() < 2) return;
    for (const Vertex& p : vertices)
        if (!std::isfinite(p.x) || !std::isfinite(p.y)) return;

    if (fill && vertices.size() >= 3) fillPolygon(ctx, vertices, *fill);

    if (stroke && strokeWidth > 0) {
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            const Vertex& a = vertices[i];
            const Vertex& b = vertices[(i + 1) % vertices.size()];
            Line edge{a.x, a.y, b.x, b.y, *stroke, strokeWidth};
            edge.draw(ctx);
        }
    }
}

}  // namespace gfx
