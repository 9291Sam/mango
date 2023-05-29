#ifndef SRC_GAME_ENTITY_HPP
#define SRC_GAME_ENTITY_HPP

#include <gfx/renderer.hpp>
#include <util/uuid.hpp>

namespace game::entity
{
    class Entity
    {
    public:
        Entity(gfx::Renderer&);
        virtual ~Entity();

        Entity(const Entity&)             = delete;
        Entity(Entity&&)                  = delete;
        Entity& operator= (const Entity&) = delete;
        Entity& operator= (Entity&&)      = delete;

        virtual void                            tick()       = 0;
        virtual std::vector<const gfx::Object*> draw() const = 0;

        virtual explicit operator std::string () const;

    protected:
        gfx::Renderer&   renderer;
        const util::UUID uuid;
    };
} // namespace game::entity

#endif // SRC_GAME_ENTITY_HPP
