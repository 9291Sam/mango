#ifndef SRC_GAME_GAME_HPP
#define SRC_GAME_GAME_HPP

#include <gfx/camera.hpp>
#include <memory>

namespace gfx
{
    class Renderer;
}

namespace game
{
    namespace entity
    {
        class Entity;
    }

    class Game
    {
    public:

        Game(std::shared_ptr<gfx::Renderer>);
        ~Game();

        Game(const Game&)             = delete;
        Game(Game&&)                  = delete;
        Game& operator= (const Game&) = delete;
        Game& operator= (Game&&)      = delete;

        void tick();

    private:
        std::shared_ptr<gfx::Renderer>               renderer;
        std::vector<std::unique_ptr<entity::Entity>> entities;
        gfx::Camera                                  camera;
    }; // class Game

} // namespace game

#endif // SRC_GAME_GAME_HPP
