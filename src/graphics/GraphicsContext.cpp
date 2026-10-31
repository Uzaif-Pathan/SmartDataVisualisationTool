#include "graphics/GraphicsContext.h"

#include <algorithm>

namespace gfx {

GraphicsContext::GraphicsContext(Canvas& canvas) : canvas_(canvas) { clearClip(); }

void GraphicsContext::clearClip() {
    clipX0_ = 0;
    clipY0_ = 0;
    clipX1_ = canvas_.width();
    clipY1_ = canvas_.height();
}

void GraphicsContext::setClip(int x0, int y0, int x1, int y1) {
    clipX0_ = std::max(0, x0);
    clipY0_ = std::max(0, y0);
    clipX1_ = std::min(canvas_.width(), x1);
    clipY1_ = std::min(canvas_.height(), y1);
}

void GraphicsContext::setPixel(int x, int y, const Color& color) {
    if (x < clipX0_ || x >= clipX1_ || y < clipY0_ || y >= clipY1_) return;
    canvas_.setPixel(x, y, color);
}

void GraphicsContext::hspan(int xStart, int xEnd, int y, const Color& color) {
    if (y < clipY0_ || y >= clipY1_) return;
    xStart = std::max(xStart, clipX0_);
    xEnd = std::min(xEnd, clipX1_);
    for (int x = xStart; x < xEnd; ++x) canvas_.setPixel(x, y, color);
}

void GraphicsContext::fillRect(int x0, int y0, int x1, int y1, const Color& color) {
    const int yStart = std::max(y0, clipY0_);
    const int yEnd = std::min(y1, clipY1_);
    for (int y = yStart; y < yEnd; ++y) hspan(x0, x1, y, color);
}

}  // namespace gfx
