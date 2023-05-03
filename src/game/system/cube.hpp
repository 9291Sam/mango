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
    class Cube final : public System
    {
    public:

        Cube(std::shared_ptr<gfx::Renderer>);
        ~Cube() override = default;

        void tick(float) override;

        [[nodiscard]] std::vector<const gfx::Object*> lend() const override;

    private:
        gfx::Object object;
    };
} // namespace game::system

#endif // SRC_GAME_SYSTEM_CUBE_HPP
