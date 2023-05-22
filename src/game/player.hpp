#ifndef SRC_GAME_PLAYER_HPP
#define SRC_GAME_PLAYER_HPP

#include <gfx/camera.hpp>

namespace gfx
{
    class Renderer;
}

namespace game
{
    class Player final
    {
    public:

        Player(gfx::Renderer&, glm::vec3 position);
        ~Player() = default;

        Player(const Player&)             = delete;
        Player(Player&&)                  = default;
        Player& operator= (const Player&) = delete;
        Player& operator= (Player&&)      = default;

        void         tick();
        gfx::Camera& getCamera();

    private:
        gfx::Renderer& renderer;
        gfx::Camera    camera;
    };
} // namespace game

// TODO has a camera

#endif // SRC_GAME_PLAYER_HPP