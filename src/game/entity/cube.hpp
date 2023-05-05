#ifndef SRC_GAME_ENTITY_CUBE_HPP
#define SRC_GAME_ENTITY_CUBE_HPP

#include "entity.hpp"
#include <memory>

namespace gfx
{
    class Renderer;
}

namespace game::entity
{
    class Cube final : public Entity
    {
    public:

        Cube(std::shared_ptr<gfx::Renderer>);
        ~Cube() override = default;

        void tick(float) override;

        [[nodiscard]] std::vector<const gfx::Object*> lend() const override;

    private:
        gfx::Object object;
    };
} // namespace game::entity

#endif // SRC_GAME_ENTITY_CUBE_HPP
