#include "renderer.hpp"
#include "util/log.hpp"
#include "vulkan/includes.hpp"

namespace gfx
{
    Renderer::Renderer()
        : window {
            {1'200, 1'200},
            "Mango"
    }
    {
        util::logLog("Renderer initialization complete");
    }

    Renderer::~Renderer() {}
} // namespace gfx
