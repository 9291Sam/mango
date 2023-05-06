#ifndef SRC_GAME_WORLD_WORLD_HPP
#define SRC_GAME_WORLD_WORLD_HPP

#include "voxel.hpp"
#include <gfx/object.hpp>
#include <optional>
#include <vector>

namespace gfx
{
    class Renderer;
    class Object;
} // namespace gfx

namespace game::world
{
    class World
    {
    public:

        World(
            std::shared_ptr<gfx::Renderer>,
            std::optional<std::size_t> seed = std::nullopt);
        ~World();

        [[nodiscard]] std::vector<const gfx::Object*> lend() const;

    private:
        std::shared_ptr<gfx::Renderer> renderer;
        VoxelCube                      voxels;
        gfx::Object                    object;
    };
} // namespace game::world

#endif // SRC_GAME_WORLD_WORLD_HPP