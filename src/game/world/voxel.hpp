#ifndef SRC_GAME_WORLD_VOXEL_HPP
#define SRC_GAME_WORLD_VOXEL_HPP

#include <gfx/vulkan/includes.hpp>
// TODO: migrate to a seperate header for just LA stuff

namespace game::world
{
    // TODO: migrate to std::uint8_t
    struct Voxel
    {
        glm::vec4 linear_color;
    };
} // namespace game::world

#endif // SRC_GAME_WORLD_VOXEL_HPP