#ifndef SRC_GAME_WORLD_VOXEL__TREE_HPP
#define SRC_GAME_WORLD_VOXEL__TREE_HPP

#include "gfx/vulkan/gpu_data.hpp"
#include <bit>
#include <gfx/vulkan/includes.hpp>
#include <util/misc.hpp>
#include <variant>

// TODO: add floating origin
// TODO: I like the idea of a fixed 65535^3 volume and you just have 27 of them
// loaded in around the player?

namespace game::world
{
    class VoxelOctree
    {
    public:
        // this is the number of VoxelVolumes on on axis
        // 4096^3 tree
        static constexpr std::size_t Extent {256}; // TOTAL EXTENT -128 -> +127
        static constexpr std::size_t TraversalSteps {util::log_2(Extent)};

        struct Voxel // TODO: migrate to std::uint8_t sRGB
        {
            glm::vec4 color;

            bool shouldDraw() const;
        };

        struct Position // TODO: make into a 16 bit integer for savings?
        {
            std::int32_t x;
            std::int32_t y;
            std::int32_t z;
        };

        enum class Octant : std::uint_fast8_t
        {
            pXpYpZ = 0,
            pXpYnZ = 1,
            pXnYpZ = 2,
            pXnYnZ = 3,
            nXpYpZ = 4,
            nXpYnZ = 5,
            nXnYpZ = 6,
            nXnYnZ = 7,
        };

        // TODO: bad name bad design
        struct VoxelVolume
        {
            static constexpr std::size_t Size {16};
            static constexpr std::size_t XMultiplier {1};
            static constexpr std::size_t YMultiplier {Size};
            static constexpr std::size_t ZMultiplier {Size * Size};

            VoxelVolume();

            std::pair<
                std::vector<gfx::vulkan::Vertex>,
                std::vector<gfx::vulkan::Index>>
            draw() const;

            Voxel& access(Position);
            Voxel  access(Position) const;

        private:
            std::array<Voxel, Size * Size * Size> storage;
        };
    public:
        explicit VoxelOctree() = default;
        ~VoxelOctree()         = default;

        VoxelOctree(const VoxelOctree&)             = delete;
        VoxelOctree(VoxelOctree&&)                  = delete;
        VoxelOctree& operator= (const VoxelOctree&) = delete;
        VoxelOctree& operator= (VoxelOctree&&)      = delete;

        Voxel& access(Position);

        std::pair<
            std::vector<gfx::vulkan::Vertex>,
            std::vector<gfx::vulkan::Index>>
        draw() const;
        // TODO; draw from vertices

    private:
        struct Node
        {
            Node();

            std::variant<
                std::unique_ptr<VoxelVolume>,
                std::array<std::unique_ptr<Node>, 8>>
                data;
        };

        // me when I dont have recursive lambdas
        friend void drawImpl(
            std::vector<gfx::vulkan::Vertex>&,
            std::vector<gfx::vulkan::Index>&,
            const VoxelOctree::Node*,
            glm::vec3,
            std::size_t);

        Node parent;
    };
} // namespace game::world

#endif // SRC_GAME_WORLD_VOXEL__TREE_HPP