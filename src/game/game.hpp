#ifndef SRC_GAME_GAME_HPP
#define SRC_GAME_GAME_HPP

#include "game/player.hpp"
#include "world/voxel_octree.hpp"
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
    } // namespace entity

    class Game
    {
    public:

        Game(gfx::Renderer&);
        ~Game();

        Game(const Game&)             = delete;
        Game(Game&&)                  = delete;
        Game& operator= (const Game&) = delete;
        Game& operator= (Game&&)      = delete;

        void tick();

    private:
        gfx::Renderer&                               renderer;
        Player                                       player;
        std::vector<std::unique_ptr<entity::Entity>> entities;
        world::VoxelOctree                           world;
    };
} // namespace game

#endif // SRC_GAME_GAME_HPP