#ifndef SRC_GAME_WORLD_VOXEL__OCTREE_HPP
#define SRC_GAME_WORLD_VOXEL__OCTREE_HPP

#include "gfx/vulkan/gpu_data.hpp"
#include <gfx/vulkan/includes.hpp>
#include <memory>
#include <util/misc.hpp>
#include <variant>

namespace game::world
{
    class VoxelVolume;

    struct Node
    {
        Node();

        std::variant<
            std::unique_ptr<VoxelVolume>,
            std::array<std::unique_ptr<Node>, 8>>
            data;
    };

    struct Voxel
    {
        Voxel();
        Voxel(glm::vec4);

        glm::vec4 color;

        bool shouldDraw() const;
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

    struct Position
    {
        std::int32_t x;
        std::int32_t y;
        std::int32_t z;

        Position operator- () const;
        Position operator- (Position other) const;
        Position operator+ (Position other) const;

        operator glm::vec3 () const;

        operator std::string () const;
    };

    struct VoxelVolume
    {
        static constexpr std::size_t Extent {16};
        static constexpr std::size_t Minimum {0};
        static constexpr std::size_t Maximum {Extent - 1};

        VoxelVolume(Position localOffset);

        // TODO: remove once working
        Voxel& accessFromGlobalPosition(Position globalPosition);

        void drawToVectors(
            std::vector<gfx::vulkan::Vertex>&,
            std::vector<gfx::vulkan::Index>&);

    private:
        Voxel& accessFromLocalPosition(Position localPosition);

        Position local_offset;
        std::array<std::array<std::array<Voxel, Extent>, Extent>, Extent>
            storage;
    };

    class VoxelOctree
    {
    public:
        static constexpr std::size_t TraversalSteps {8};

        static constexpr std::size_t VolumeExtent {
            util::exp(static_cast<std::size_t>(2), TraversalSteps)
            * VoxelVolume::Extent};

        static constexpr std::int32_t VolumeMinimum {
            -static_cast<std::int32_t>(VolumeExtent / 2)};

        static constexpr std::int32_t VolumeMaximum {
            static_cast<std::int32_t>((VolumeExtent / 2) - 1)};

        static constexpr std::int32_t VoxelMinimum {
            -static_cast<std::int32_t>((VolumeExtent * VoxelVolume::Extent))
            / 2};

        static constexpr std::int32_t VoxelMaximum {
            (static_cast<std::int32_t>(VolumeExtent * VoxelVolume::Extent) / 2)
            - 1};
    public:
        explicit VoxelOctree() = default;
        ~VoxelOctree()         = default;

        VoxelOctree(const VoxelOctree&)             = delete;
        VoxelOctree(VoxelOctree&&)                  = delete;
        VoxelOctree& operator= (const VoxelOctree&) = delete;
        VoxelOctree& operator= (VoxelOctree&&)      = delete;

        std::pair<
            std::vector<gfx::vulkan::Vertex>,
            std::vector<gfx::vulkan::Index>>
        draw() const;

        Voxel& access(Position);


    private:
        Node parent;
    };
} // namespace game::world

#endif // SRC_GAME_WORLD_VOXEL__OCTREE_HPP