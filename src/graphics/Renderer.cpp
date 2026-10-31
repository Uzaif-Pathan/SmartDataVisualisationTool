#include "graphics/Renderer.h"

namespace gfx {

Renderer::Renderer(int width, int height, const Color& background)
    : canvas_(width, height, background), context_(canvas_) {}

}  // namespace gfx
