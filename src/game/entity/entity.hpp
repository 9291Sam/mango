#ifndef SRC_GAME_ENTITY_ENTITY_HPP
#define SRC_GAME_ENTITY_ENTITY_HPP

#include <gfx/object.hpp>
#include <memory>
#include <span>
#include <unordered_map>
#include <util/uuid.hpp>
#include <vector>

namespace gfx
{
    class Renderer;
} // namespace gfx

namespace game::entity
{
    class Entity
    {
    public:
        Entity(std::shared_ptr<gfx::Renderer>);
        virtual ~Entity();

        Entity(const Entity&)             = delete;
        Entity(Entity&&)                  = delete;
        Entity& operator= (const Entity&) = delete;
        Entity& operator= (Entity&&)      = delete;

        virtual void tick(float deltaTime)                                 = 0;
        [[nodiscard]] virtual std::vector<const gfx::Object*> lend() const = 0;

        [[nodiscard]] virtual explicit operator std::string () const;

    protected:
        std::shared_ptr<gfx::Renderer> renderer;
        const util::UUID               uuid;
    };

} // namespace game::entity

#endif // SRC_GAME_ENTITY_ENTITY_HPP
