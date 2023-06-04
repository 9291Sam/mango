#include "voxel_octree.hpp"
#include "game/world/voxel_octree.hpp"
#include "gfx/vulkan/gpu_data.hpp"
#include "util/misc.hpp"
#include <algorithm>
#include <ranges>
#include <type_traits>
#include <util/log.hpp>
#include <variant>

namespace game::world
{
    VoxelOctree::Octant getOctantOfPosition(VoxelOctree::Position position)
    {
        if (position.x >= 0)
        {
            if (position.y >= 0)
            {
                if (position.z >= 0)
                {
                    return VoxelOctree::Octant::pXpYpZ;
                }
                else
                {
                    return VoxelOctree::Octant::pXpYnZ;
                }
            }
            else
            {
                if (position.z >= 0)
                {
                    return VoxelOctree::Octant::pXnYpZ;
                }
                else
                {
                    return VoxelOctree::Octant::pXnYnZ;
                }
            }
        }
        else
        {
            if (position.y >= 0)
            {
                if (position.z >= 0)
                {
                    return VoxelOctree::Octant::nXpYpZ;
                }
                else
                {
                    return VoxelOctree::Octant::nXpYnZ;
                }
            }
            else
            {
                if (position.z >= 0)
                {
                    return VoxelOctree::Octant::nXnYpZ;
                }
                else
                {
                    return VoxelOctree::Octant::nXnYnZ;
                }
            }
        }
    }

    VoxelOctree::Position
    getOffsetPositionByOctant(VoxelOctree::Octant octant, std::int32_t size)
    {
        using enum VoxelOctree::Octant;

        switch (octant)
        {
        case pXpYpZ:
            return VoxelOctree::Position {.x {size}, .y {size}, .z {size}};

        case pXpYnZ:
            return VoxelOctree::Position {.x {size}, .y {size}, .z {-size}};

        case pXnYpZ:
            return VoxelOctree::Position {.x {size}, .y {-size}, .z {size}};

        case pXnYnZ:
            return VoxelOctree::Position {.x {size}, .y {-size}, .z {-size}};

        case nXpYpZ:
            return VoxelOctree::Position {.x {-size}, .y {size}, .z {size}};

        case nXpYnZ:
            return VoxelOctree::Position {.x {-size}, .y {size}, .z {-size}};

        case nXnYpZ:
            return VoxelOctree::Position {.x {-size}, .y {-size}, .z {size}};

        case nXnYnZ:
            return VoxelOctree::Position {.x {-size}, .y {-size}, .z {-size}};
        }

        util::panic("Unreachable enum {}", util::toUnderlyingType(octant));
    }

    glm::vec3 getOffsetPositionByOctant(VoxelOctree::Octant octant, float size)
    {
        using enum VoxelOctree::Octant;

        switch (octant)
        {
        case pXpYpZ:
            return glm::vec3 {size, size, size};

        case pXpYnZ:
            return glm::vec3 {size, size, -size};

        case pXnYpZ:
            return glm::vec3 {size, -size, size};

        case pXnYnZ:
            return glm::vec3 {size, -size, -size};

        case nXpYpZ:
            return glm::vec3 {-size, size, size};

        case nXpYnZ:
            return glm::vec3 {-size, size, -size};

        case nXnYpZ:
            return glm::vec3 {-size, -size, size};

        case nXnYnZ:
            return glm::vec3 {-size, -size, -size};
        }

        util::panic("Unreachable enum {}", util::toUnderlyingType(octant));
    }

    void generateIndiciesToGetToVoxelImpl(
        std::vector<std::size_t>& indicies,
        VoxelOctree::Position     position,
        std::size_t               size)
    {
        // TODO: move to switch
        if (size < 2)
        {
            util::panic(
                "Invalid generateIndiciesToGetToVoxelImpl size {}", size);
        }

        if (size == 2)
        {
            indicies.push_back(
                static_cast<std::size_t>(getOctantOfPosition(position)));
            return;
        }

        VoxelOctree::Octant o = getOctantOfPosition(position);

        indicies.push_back(static_cast<std::size_t>(o));

        VoxelOctree::Position offset =
            getOffsetPositionByOctant(o, static_cast<std::int32_t>(size / 2));

        // TODO: should this be +
        generateIndiciesToGetToVoxelImpl(indicies, position - offset, size / 2);
    }

    std::array<
        std::underlying_type_t<VoxelOctree::Octant>,
        VoxelOctree::TraversalSteps>
    getPathToVolume(VoxelOctree::Position position)
    {
        std::vector<std::size_t> indices {};

        generateIndiciesToGetToVoxelImpl(
            indices,
            position,
            VoxelOctree::VoxelVolume::Size * VoxelOctree::Extent);

        std::array<
            std::underlying_type_t<VoxelOctree::Octant>,
            VoxelOctree::TraversalSteps>
            output;

        // TODO: trim the excess lmfao
        for (std::size_t i = 0; i < output.size(); ++i)
        {
            output[i] = indices.at(i);
        }

        return output;
    }

    // std::array<
    //     std::underlying_type_t<VoxelOctree::Octant>,
    //     VoxelOctree::TraversalSteps>
    // getPathToVolume(VoxelOctree::Position position)
    // {
    //     std::size_t nextIndexToPopulate = 0;
    //     std::array<
    //         std::underlying_type_t<VoxelOctree::Octant>,
    //         VoxelOctree::TraversalSteps>
    //         output {};

    //     std::int32_t size = static_cast<std::int32_t>(
    //         VoxelOctree::VoxelVolume::Size * VoxelOctree::Extent);

    //     while (nextIndexToPopulate != VoxelOctree::TraversalSteps)
    //     {
    //         VoxelOctree::Octant octant = getOctantOfPosition(position);

    //         output[nextIndexToPopulate] = util::toUnderlyingType(octant);

    //         VoxelOctree::Position difference =
    //             getOffsetPositionByOctant(octant, size / 2);

    //         size = size / 2;

    //         position.x -= difference.x;
    //         position.y -= difference.y;
    //         position.z -= difference.z;

    //         ++nextIndexToPopulate;
    //     }

    //     return output;
    // }

    void generateTrianglesFromVoxel(
        std::vector<gfx::vulkan::Vertex>& vertices,
        std::vector<gfx::vulkan::Index>&  indices,
        glm::vec3                         position,
        VoxelOctree::Voxel                voxel,
        float                             size)
    {
        if (!voxel.shouldDraw())
        {
            return;
        }

        const std::array<gfx::vulkan::Vertex, 8> cube_vertices {
            gfx::vulkan::Vertex {
                .position {-0.5f, -0.5f, -0.5f},
                .color {voxel.color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-0.5f, -0.5f, 0.5f},
                .color {voxel.color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-0.5f, 0.5f, -0.5f},
                .color {voxel.color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-0.5f, 0.5f, 0.5f},
                .color {voxel.color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {0.5f, -0.5f, -0.5f},
                .color {voxel.color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {0.5f, -0.5f, 0.5f},
                .color {voxel.color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {0.5f, 0.5f, -0.5f},
                .color {voxel.color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {0.5f, 0.5f, 0.5f},
                .color {voxel.color},
                .normal {},
                .uv {},
            },
        };

        std::array<gfx::vulkan::Index, 36> cube_indices {
            6, 2, 7, 2, 3, 7, 0, 4, 5, 1, 0, 5, 0, 2, 6, 4, 0, 6,
            3, 1, 7, 1, 5, 7, 2, 0, 3, 0, 1, 3, 4, 6, 7, 5, 4, 7};

        const std::size_t indices_offset = vertices.size();

        for (gfx::vulkan::Index& i : cube_indices)
        {
            i += indices_offset;
        }

        for (gfx::vulkan::Vertex vertex : cube_vertices)
        {
            vertex.position *= size;
            vertex.position += position;

            vertices.push_back(vertex);
        }

        for (gfx::vulkan::Index index : cube_indices)
        {
            indices.push_back(index);
        }
    }

    bool VoxelOctree::Voxel::shouldDraw() const
    {
        if (this->color.a == 0.0f)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    VoxelOctree::Position::operator std::string () const
    {
        return fmt::format(
            "Position X: {} | Y: {} | Z: {}", this->x, this->y, this->z);
    }

    VoxelOctree::VoxelVolume::VoxelVolume()
        : storage {}
    {
        std::ranges::fill(this->storage, Voxel {{0.0f, 0.0f, 0.0f, 0.00f}});
    }

    std::pair<std::vector<gfx::vulkan::Vertex>, std::vector<gfx::vulkan::Index>>
    VoxelOctree::VoxelVolume::draw() const
    {
        std::vector<gfx::vulkan::Vertex> outputVertices;
        std::vector<gfx::vulkan::Index>  outputIndices;

        auto iterator = std::ranges::iota_view {
            0, static_cast<std::int32_t>(VoxelVolume::Size)};

        for (std::int32_t x : iterator)
        {
            for (std::int32_t y : iterator)
            {
                for (std::int32_t z : iterator)
                {
                    generateTrianglesFromVoxel(
                        outputVertices,
                        outputIndices,
                        glm::vec3 {x, y, z},
                        this->access(Position {x, y, z}),
                        1.0f);

                    // util::logTrace(
                    //     "Generated Voxel at position {} | {}",
                    //     glm::to_string(
                    //         outputVertices.at(outputVertices.size() - 1)
                    //             .position),
                    //     static_cast<std::string>(Position {x, y, z}));
                }
            }
        }

        return std::make_pair(
            std::move(outputVertices), std::move(outputIndices));
    }

    VoxelOctree::Voxel&
    VoxelOctree::VoxelVolume::access(VoxelOctree::Position position)
    {
        util::assertFatal(
            position.x >= 0 && position.x < VoxelVolume::Size,
            "X: {} is out of bounds!",
            position.x);
        util::assertFatal(
            position.y >= 0 && position.y < VoxelVolume::Size,
            "Y: {} is out of bounds!",
            position.y);
        util::assertFatal(
            position.z >= 0 && position.z < VoxelVolume::Size,
            "Z: {} is out of bounds!",
            position.z);

        return this->storage
            [position.x * VoxelVolume::XMultiplier
             + position.y * VoxelVolume::YMultiplier
             + position.z * VoxelVolume::ZMultiplier];
    }

    VoxelOctree::Voxel
    VoxelOctree::VoxelVolume::access(VoxelOctree::Position position) const
    {
        return const_cast<VoxelVolume*>(this)->access(position);
    }

    auto VoxelOctree::access(Position position) -> Voxel&
    {
        const std::int32_t extent =
            (VoxelOctree::Extent * VoxelVolume::Size) / 2;

        util::assertFatal(
            position.x >= -extent && position.x < extent,
            "X: {} is out of bounds!",
            position.x);
        util::assertFatal(
            position.y >= -extent && position.y < extent,
            "Y: {} is out of bounds!",
            position.y);
        util::assertFatal(
            position.z >= -extent && position.z < extent,
            "Z: {} is out of bounds!",
            position.z);

        Node*        workingNode = &this->parent;
        VoxelVolume* localVolume = nullptr;

        // HERE
        std::array<
            std::underlying_type_t<VoxelOctree::Octant>,
            VoxelOctree::TraversalSteps>
            path = getPathToVolume(position);

        // util::logTrace(
        //     "Position: {} | Path: {}",
        //     static_cast<std::string>(position),
        //     util::toString(path));

        for (std::size_t i = 0; i < path.size(); ++i)
        {
            // population routine of Volume
            // if last index
            // If this is the last index and there isn't a voxelVolume here
            // we need to create one, do so
            if (i == path.size() - 1
                && !std::holds_alternative<std::unique_ptr<VoxelVolume>>(
                    workingNode->data))
            {
                workingNode->data = std::make_unique<VoxelVolume>();
            }

            std::visit(
                util::VariantHelper {
                    [&](const std::unique_ptr<VoxelVolume>& volume)
                    {
                        localVolume = volume.get();
                    },
                    [&](std::array<std::unique_ptr<Node>, 8>& nodes)
                    {
                        if (nodes[path[i]] == nullptr)
                        {
                            // util::logTrace("Created new node!");
                            nodes[path[i]] = std::make_unique<Node>();
                        }

                        workingNode = nodes[path[i]].get();
                    }},
                workingNode->data);
        }

        std::int32_t i32Size = static_cast<std::int32_t>(VoxelVolume::Size);

        // Position localPosition {
        //     .x {((position.x % i32Size) + i32Size) % i32Size},
        //     .y {((position.y % i32Size) + i32Size) % i32Size},
        //     .z {((position.z % i32Size) + i32Size) % i32Size},
        // };

        Position localPosition {
            .x {((-(position.x % i32Size)) + i32Size) % i32Size},
            .y {((-(position.y % i32Size)) + i32Size) % i32Size},
            .z {((-(position.z % i32Size)) + i32Size) % i32Size},
        };

        // util::logTrace(
        //     "GlobalPosition: {} | Path: {} | LocalPosition: {}",
        //     static_cast<std::string>(position),
        //     util::toString(path),
        //     static_cast<std::string>(localPosition));

        return localVolume->access(localPosition);
    }

    void drawImpl(
        std::vector<gfx::vulkan::Vertex>& outVertices,
        std::vector<gfx::vulkan::Index>&  outIndices,
        const VoxelOctree::Node*          node,
        glm::vec3                         center,
        std::size_t                       size)
    {
        std::visit(
            util::VariantHelper {
                [&](const std::unique_ptr<VoxelOctree::VoxelVolume>& volume)
                {
                    auto [vertices, indices] = volume->draw();

                    // TODO: code duplication!
                    const std::size_t indicesOffset = outVertices.size();

                    for (gfx::vulkan::Vertex& vertex : vertices)
                    {
                        vertex.position += center;

                        // util::logTrace(
                        //     "Vertex at {} | {}",
                        //     glm::to_string(vertex.position),
                        //     glm::to_string(center));

                        outVertices.push_back(vertex);
                    }

                    for (gfx::vulkan::Index index : indices)
                    {
                        outIndices.push_back(index + indicesOffset);
                    }
                    // util::logTrace(
                    //     "Drew voxel volume @ {}", glm::to_string(center));
                },
                [&](const std::array<std::unique_ptr<VoxelOctree::Node>, 8>&
                        nodes)
                {
                    for (std::size_t i = 0; i < nodes.size(); ++i)
                    {
                        if (nodes[i] != nullptr)
                        {
                            glm::vec3 rawOffset = getOffsetPositionByOctant(
                                static_cast<VoxelOctree::Octant>(i),
                                static_cast<float>(size) / 8.0f);

                            glm::vec3 totalOffset = center + rawOffset;

                            // util::logTrace(
                            //     "Offset: {} | Center: {} | Total: {}",
                            //     glm::to_string(rawOffset),
                            //     glm::to_string(center),
                            //     glm::to_string(totalOffset));

                            drawImpl(
                                outVertices,
                                outIndices,
                                nodes[i].get(),
                                totalOffset,
                                size / 2);

                            // TODO: the drawing seems to be fine, its the
                            // insertion routine
                        }
                        else
                        {
                            // util::logTrace(
                            //     "Early discard of octant @ {}",
                            //     glm::to_string(center));
                        }
                    }
                }},
            node->data);
    }

    std::pair<std::vector<gfx::vulkan::Vertex>, std::vector<gfx::vulkan::Index>>
    VoxelOctree::draw() const
    {
        std::vector<gfx::vulkan::Vertex> outputVertices;
        std::vector<gfx::vulkan::Index>  outputIndices;

        drawImpl(
            outputVertices,
            outputIndices,
            &this->parent,
            glm::vec3 {0.0f, 0.0f, 0.0f},
            VoxelOctree::Extent * VoxelOctree::VoxelVolume::Size);

        return std::make_pair(
            std::move(outputVertices), std::move(outputIndices));
    }

    VoxelOctree::Node::Node()
        : data {std::array<std::unique_ptr<Node>, 8> {nullptr}}
    {}

} // namespace game::world