#ifndef SRC_GAME_SYSTEM_CUBE_HPP
#define SRC_GAME_SYSTEM_CUBE_HPP

#include "system.hpp"
#include <memory>

namespace gfx
{
    class Renderer;
}

namespace game::system
{
    class Cube : public ObjectSystem
    {
    public:
        Cube(std::shared_ptr<gfx::Renderer>);
        ~Cube() override = default;

        void tick(float) override;

        [[nodiscard]] std::vector<const gfx::Object*> lend() const override;
    };
} // namespace game::system

#endif // SRC_GAME_SYSTEM_CUBE_HPP
