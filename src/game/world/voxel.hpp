#ifndef SRC_GAME_WORLD_VOXEL_HPP
#define SRC_GAME_WORLD_VOXEL_HPP

#include <cstdint>
#include <gfx/transform.hpp>
#include <gfx/vulkan/gpu_data.hpp>
#include <limits>
#include <optional>
#include <span>
#include <util/misc.hpp>
#include <utility>
#include <vector>

namespace gfx
{
    class Renderer;

    namespace vulkan
    {
        using Index = std::uint32_t;

        struct Vertex;
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

        // TODO: make vec4 for transperency
        constexpr inline glm::vec3 getFloatColors() const
        {
            return glm::vec3 {
                util::map<float>(
                    static_cast<float>(this->r), 0.0f, 255.0f, 0.0f, 1.0f),
                util::map<float>(
                    static_cast<float>(this->g), 0.0f, 255.0f, 0.0f, 1.0f),
                util::map<float>(
                    static_cast<float>(this->b), 0.0f, 255.0f, 0.0f, 1.0f)};
        }
    };

    class VoxelCube
    {
    public:

        VoxelCube(
            glm::vec3            position,
            std::size_t          sideDimension,
            std::optional<Voxel> fillVoxel = std::nullopt);
        ~VoxelCube() = default;

        [[nodiscard]] std::pair<
            std::vector<gfx::vulkan::Vertex>,
            std::vector<gfx::vulkan::Index>>
        getVerticesAndIndices() const;

        Voxel& at(std::size_t x, std::size_t y, std::size_t z);
        Voxel  at(std::size_t x, std::size_t y, std::size_t z) const;
        bool   voxelExistsAt(std::size_t x, std::size_t y, std::size_t z) const;

        glm::vec3 position;
    private:
        std::size_t        side_dimension;
        std::vector<Voxel> voxels;
    };

} // namespace game::world

#endif // SRC_GAME_WORLD_VOXEL_HPP
