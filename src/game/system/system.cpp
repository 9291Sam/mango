#include "system.hpp"
#include <gfx/renderer.hpp>

namespace game::system
{
    System::~System() {}

    ObjectSystem::ObjectSystem(std::shared_ptr<gfx::Renderer> renderer_)
        : renderer {std::move(renderer_)}
    {}

    ObjectSystem::~ObjectSystem() {}
} // namespace game::system
