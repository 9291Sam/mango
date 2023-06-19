#include "entity.hpp"
#include <fmt/format.h>

namespace game::entity
{

    Entity::Entity(const gfx::Renderer& renderer_)
        : renderer {renderer_}
        , uuid {}
    {}

    Entity::operator std::string () const
    {
        return fmt::format("Entity {}", static_cast<std::string>(this->uuid));
    }

} // namespace game::entity
