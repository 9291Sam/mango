#ifndef SRC_GAME_WORLD_VOXEL_HPP
#define SRC_GAME_WORLD_VOXEL_HPP

#include <cstdint>
#include <gfx/transform.hpp>
#include <gfx/vulkan/gpu_data.hpp>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace gfx
{
    class Renderer;

    namespace vulkan
    {
        using Index = std::uint32_t;

        class Vertex;
    } // namespace vulkan
} // namespace gfx

namespace game::world
{
    struct Voxel
    {
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        std::uint8_t a;
    };

    // TODO: add a transform offset
    class VoxelCube
    {
    public:

        VoxelCube(
            gfx::Transform,
            std::size_t          sideDimension,
            std::optional<Voxel> fillVoxel = std::nullopt);
        ~VoxelCube();

        [[nodiscard]] std::pair<
            std::vector<gfx::vulkan::Vertex>,
            std::vector<gfx::vulkan::Index>>
        getVerticesAndIndicies() const;

        Voxel& at(std::size_t x, std::size_t y, std::size_t z);
        Voxel  at(std::size_t x, std::size_t y, std::size_t z) const;

        gfx::Transform transform;
    private:
        std::size_t        side_dimension;
        std::vector<Voxel> voxels;
    };

} // namespace game::world

#endif // SRC_GAME_WORLD_VOXEL_HPP