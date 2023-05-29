#ifndef SRC_GAME_PLAYER_HPP
#define SRC_GAME_PLAYER_HPP

#include <gfx/camera.hpp>

namespace gfx
{
    class Renderer;
}

namespace game
{
    class Player
    {
    public:

        Player(gfx::Renderer&, glm::vec3 position);
        ~Player() = default;

        Player(const Player&)             = delete;
        Player(Player&&)                  = delete;
        Player& operator= (const Player&) = delete;
        Player& operator= (Player&&)      = delete;

        void         tick();
        gfx::Camera& getCamera();

    private:
        gfx::Renderer& renderer;
        gfx::Camera    camera;
    };
} // namespace game

#endif // SRC_GAME_PLAYER_HPP
