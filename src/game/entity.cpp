#include "entity.hpp"
#include <fmt/format.h>

namespace game
{

    Entity::Entity(gfx::Renderer& renderer_)
        : renderer {renderer_}
        , uuid {}
    {}

    Entity::~Entity() {}

    Entity::operator std::string () const
    {
        return fmt::format("Entity {}", static_cast<std::string>(this->uuid));
    }

} // namespace game
