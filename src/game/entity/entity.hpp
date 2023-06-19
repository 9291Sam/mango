#ifndef SRC_GAME_ENTITY_HPP
#define SRC_GAME_ENTITY_HPP

#include <gfx/renderer.hpp>
#include <util/uuid.hpp>

namespace game::entity
{
    class Entity
    {
    public:
        explicit Entity(const gfx::Renderer&);
        virtual ~Entity() = default;

        Entity(const Entity&)             = delete;
        Entity(Entity&&)                  = delete;
        Entity& operator= (const Entity&) = delete;
        Entity& operator= (Entity&&)      = delete;

        virtual void                                          tick()       = 0;
        [[nodiscard]] virtual std::vector<const gfx::Object*> draw() const = 0;

        virtual explicit operator std::string () const;

    protected:
        const gfx::Renderer& renderer;
        const util::UUID     uuid;
    };
} // namespace game::entity

#endif // SRC_GAME_ENTITY_HPP
