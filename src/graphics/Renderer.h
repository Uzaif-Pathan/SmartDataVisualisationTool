#pragma once
#include <string>

#include "graphics/Canvas.h"
#include "graphics/Color.h"
#include "graphics/GraphicsContext.h"

namespace gfx {

// Owns a Canvas and the pen that draws on it.  Charts will draw through this.
//   Renderer r(800, 600);
//   r.draw(someRectangle);          // works for any shape with a draw() method
//   r.savePng("output/chart.png");
class Renderer {
public:
    Renderer(int width, int height, const Color& background = colors::White);

    // Not copyable: the pen holds a reference to this object's canvas.
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    int width() const { return canvas_.width(); }
    int height() const { return canvas_.height(); }
    void clear(const Color& color) { canvas_.clear(color); }

    template <typename Shape>
    void draw(const Shape& shape) { shape.draw(context_); }

    GraphicsContext& context() { return context_; }
    const Canvas& canvas() const { return canvas_; }
    bool savePng(const std::string& path) const { return canvas_.savePng(path); }

private:
    Canvas canvas_;            // must be declared before context_
    GraphicsContext context_;
};

}  // namespace gfx
