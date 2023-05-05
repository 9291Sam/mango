#include "entity.hpp"
#include <fmt/format.h>
#include <gfx/renderer.hpp>
#include <util/threads.hpp>

namespace game::entity
{
    Entity::Entity(std::shared_ptr<gfx::Renderer> renderer_)
        : renderer {std::move(renderer_)}
        , uuid {}
    {}

    Entity::~Entity() {}

    Entity::operator std::string () const
    {
        return fmt::format(
            "Entity: UUID: {}", static_cast<std::string>(this->uuid));
    }
} // namespace game::entity
