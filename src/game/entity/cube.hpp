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

        Cube(gfx::Renderer&, glm::vec3 position);
        ~Cube() override = default;

        void                                          tick() override;
        [[nodiscard]] std::vector<const gfx::Object*> draw() const override;

    private:
        gfx::SimpleTriangulatedObject object;
    };
} // namespace game::entity

#endif // SRC_GAME_ENTITY_CUBE_HPP
