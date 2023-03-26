#ifndef SRC_GFX_RENDERER_HPP
#define SRC_GFX_RENDERER_HPP

#include "window.hpp"

namespace gfx
{
    class Renderer
    {
    public:
        Renderer();

    private:
        Window window;
    }; // class Renderer
} // namespace gfx

#endif // SRC_GFX_RENDERER_HPP
