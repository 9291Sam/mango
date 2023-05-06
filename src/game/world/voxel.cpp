#include "voxel.hpp"
#include <numeric>
#include <random>
#include <ranges>
#include <util/log.hpp>
#include <util/misc.hpp>

namespace game::world
{
    VoxelCube::VoxelCube(
        gfx::Transform       transform_,
        std::size_t          sideDimension,
        std::optional<Voxel> fillVoxel)
        : transform {transform_}
        , side_dimension {sideDimension}
    {
        const std::size_t VoxelVolumeSize =
            this->side_dimension * this->side_dimension * this->side_dimension;

        if (fillVoxel.has_value())
        {
            this->voxels.resize(VoxelVolumeSize, *fillVoxel);
        }
        else
        {
            this->voxels.resize(VoxelVolumeSize);
        }
    }

    VoxelCube::~VoxelCube() {}

    std::pair<std::vector<gfx::vulkan::Vertex>, std::vector<gfx::vulkan::Index>>
    VoxelCube::getVerticesAndIndicies() const
    {
        const float VoxelSize =
            1.0f; // TODO: add this as a configurable parameter
        const std::array<float, 2> VoxelEdgePoints {0.0f, VoxelSize};
        const auto SideIterator = std::views::iota(0UZ, this->side_dimension);

        std::vector<gfx::vulkan::Vertex> duplicatedVertices;

        for (std::size_t x : SideIterator)
        {
            for (std::size_t y : SideIterator)
            {
                for (std::size_t z : SideIterator)
                {
                    const glm::vec3 CornerPosition {
                        this->transform.translation
                        + glm::vec3 {
                            x * VoxelSize, y * VoxelSize, z * VoxelSize}};

                    for (float voxelX : VoxelEdgePoints)
                    {
                        for (float voxelY : VoxelEdgePoints)
                        {
                            for (float voxelZ : VoxelEdgePoints)
                            {
                                duplicatedVertices.push_back(
                                    gfx::vulkan::Vertex {
                                        .position {
                                            CornerPosition
                                            + glm::
                                                vec3 {voxelX, voxelY, voxelZ}},
                                        // .color {
                                        // this->at(x, y, z).r,
                                        // this->at(x, y, z).g,
                                        // this->at(x, y, z).b},
                                        .color {
                                            util::map<float>(
                                                x,
                                                0,
                                                this->side_dimension,
                                                0.0f,
                                                1.0f),
                                            util::map<float>(
                                                y,
                                                0,
                                                this->side_dimension,
                                                0.0f,
                                                1.0f),
                                            util::map<float>(
                                                z,
                                                0,
                                                this->side_dimension,
                                                0.0f,
                                                1.0f)},
                                        .normal {0.0f, 0.0f, 0.0f},
                                        .uv {
                                            0.0f,
                                            0.0f}}); // TODO: deal with normals
                                                     // and uvs :skull:
                            }
                        }
                    }
                }
            }
        }

        std::vector<gfx::vulkan::Index> indicies;
        indicies.resize(duplicatedVertices.size());

        std::iota(indicies.begin(), indicies.end(), 0);

        std::shuffle(
            indicies.begin(), indicies.end(), std::default_random_engine {});

        return {duplicatedVertices, indicies};
    }

    Voxel& VoxelCube::at(std::size_t x, std::size_t y, std::size_t z)
    {
        // clang-format off
        return this->voxels.at(
            x
            + this->side_dimension * y
            + this->side_dimension * this->side_dimension * z);
        // clang-format on
    }

    Voxel VoxelCube::at(std::size_t x, std::size_t y, std::size_t z) const
    {
        return const_cast<VoxelCube*>(this)->at(x, y, z);
    }
} // namespace game::world