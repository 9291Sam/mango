#include "system.hpp"
#include <gfx/renderer.hpp>

namespace game::system
{
    System::System(std::shared_ptr<gfx::Renderer> renderer_)
        : renderer {std::move(renderer_)}
    {}

    System::~System() {}
} // namespace game::system
