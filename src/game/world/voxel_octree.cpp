#include "voxel_octree.hpp"
#include "game/world/voxel_octree.hpp"
#include "gfx/vulkan/gpu_data.hpp"
#include "util/misc.hpp"
#include <functional>
#include <ranges>
#include <util/log.hpp>

namespace game::world
{
    Position getOffsetPositionByOctant(Octant octant, std::int32_t size)
    {
        using enum Octant;

        switch (octant)
        {
        case pXpYpZ:
            return Position {0, 0, 0};
        case pXpYnZ:
            return Position {0, 0, -size};
        case pXnYpZ:
            return Position {0, -size, 0};
        case pXnYnZ:
            return Position {0, -size, -size};
        case nXpYpZ:
            return Position {-size, 0, 0};
        case nXpYnZ:
            return Position {-size, 0, -size};
        case nXnYpZ:
            return Position {-size, -size, 0};
        case nXnYnZ:
            return Position {-size, -size, -size};
        }

        util::panic("Unreachable enum {}", util::toUnderlyingType(octant));
    }

    Node::Node()
        : data {std::array<std::unique_ptr<Node>, 8> {nullptr}}
    {}

    Voxel::Voxel()
        : color {0.0f, 0.0f, 0.0f, 0.0f}
    {}

    Voxel::Voxel(glm::vec4 color_)
        : color {color_}
    {}

    bool Voxel::shouldDraw() const
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

    Position Position::operator- () const
    {
        return Position {-this->x, -this->y, -this->z};
    }

    Position Position::operator- (Position other) const
    {
        return (*this) + -(other);
    }

    Position Position::operator+ (Position other) const
    {
        return Position {
            this->x + other.x, this->y + other.y, this->z + other.z};
    }

    Position::operator glm::vec3 () const
    {
        return glm::vec3 {
            static_cast<float>(this->x),
            static_cast<float>(this->y),
            static_cast<float>(this->z)};
    }

    Position::operator std::string () const
    {
        return fmt::format(
            "Position X: {} | Y: {} | Z: {}", this->x, this->y, this->z);
    }

    VoxelVolume::VoxelVolume(Position offset_)
        : local_offset {offset_}
        , storage {}
    {}

    Voxel& VoxelVolume::accessFromGlobalPosition(Position globalPosition)
    {
        util::logTrace(
            "Doing global lookup: {}",
            static_cast<std::string>(globalPosition));
        return this->accessFromLocalPosition(
            globalPosition - this->local_offset);
    }

    Voxel& VoxelVolume::accessFromLocalPosition(Position localPosition)
    {
        util::logTrace(
            "Doing local lookup: {}", static_cast<std::string>(localPosition));

        util::assertFatal(
            localPosition.x >= 0 && localPosition.x < VoxelVolume::Extent,
            "X: {} is out of bounds!",
            localPosition.x);
        util::assertFatal(
            localPosition.y >= 0 && localPosition.y < VoxelVolume::Extent,
            "Y: {} is out of bounds!",
            localPosition.y);
        util::assertFatal(
            localPosition.z >= 0 && localPosition.z < VoxelVolume::Extent,
            "Z: {} is out of bounds!",
            localPosition.z);

        return this->storage[localPosition.x][localPosition.y][localPosition.z];
    }

    void VoxelVolume::drawToVectors(
        std::vector<gfx::vulkan::Vertex>& outputVertices,
        std::vector<gfx::vulkan::Index>&  outputIndices)
    {
        auto iterator = std::views::iota(0, static_cast<std::int32_t>(Extent));

        for (std::int32_t localX : iterator)
        {
            for (std::int32_t localY : iterator)
            {
                for (std::int32_t localZ : iterator)
                {
                    const Voxel voxel = this->accessFromLocalPosition(
                        Position {localX, localY, localZ});

                    // TODO: replace vertex with a smaller one
                    std::array<gfx::vulkan::Vertex, 8> cubeVertices {
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

                    std::array<gfx::vulkan::Index, 36> cubeIndices {
                        6, 2, 7, 2, 3, 7, 0, 4, 5, 1, 0, 5, 0, 2, 6, 4, 0, 6,
                        3, 1, 7, 1, 5, 7, 2, 0, 3, 0, 1, 3, 4, 6, 7, 5, 4, 7};

                    const std::size_t IndicesOffset = outputVertices.size();

                    // Update positions to be aligned with the world and insert
                    // into output
                    for (gfx::vulkan::Vertex& v : cubeVertices)
                    {
                        v.position +=
                            static_cast<glm::vec3>(this->local_offset);

                        outputVertices.push_back(v);
                    }

                    // update indices to actually point to the correct index
                    for (gfx::vulkan::Index& i : cubeIndices)
                    {
                        i += static_cast<std::uint32_t>(IndicesOffset);

                        outputIndices.push_back(i);
                    }
                }
            }
        }
    }

    std::pair<std::vector<gfx::vulkan::Vertex>, std::vector<gfx::vulkan::Index>>
    VoxelOctree::draw() const
    {
        std::vector<gfx::vulkan::Vertex> outputVertices;
        std::vector<gfx::vulkan::Index>  outputIndices;

        std::function<void(const Node*)> impl;

        impl = [&](const Node* node)
        {
            std::visit(
                util::VariantHelper {
                    [&](const std::unique_ptr<VoxelVolume>& volume)
                    {
                        volume->drawToVectors(outputVertices, outputIndices);
                    },
                    [&](const std::array<std::unique_ptr<Node>, 8>& nodes)
                    {
                        for (const std::unique_ptr<Node>& n : nodes)
                        {
                            if (n != nullptr)
                            {
                                impl(n.get());
                            }
                        }
                    }},
                node->data);
        };

        impl(&this->parent);

        return std::make_pair(outputVertices, outputIndices);
    }

    Octant getOctantFromPosition(Position position)
    {
        if (position.x >= 0)
        {
            if (position.y >= 0)
            {
                if (position.z >= 0)
                {
                    return Octant::pXpYpZ;
                }
                else
                {
                    return Octant::pXpYnZ;
                }
            }
            else
            {
                if (position.z >= 0)
                {
                    return Octant::pXnYpZ;
                }
                else
                {
                    return Octant::pXnYnZ;
                }
            }
        }
        else
        {
            if (position.y >= 0)
            {
                if (position.z >= 0)
                {
                    return Octant::nXpYpZ;
                }
                else
                {
                    return Octant::nXpYnZ;
                }
            }
            else
            {
                if (position.z >= 0)
                {
                    return Octant::nXnYpZ;
                }
                else
                {
                    return Octant::nXnYnZ;
                }
            }
        }
    }

    void generateIndiciesToGetToVoxelImpl(
        std::vector<std::size_t>& indicies, Position position, std::size_t size)
    {
        // TODO: move to switch
        if (size < 2)
        {
            util::panic(
                "Invalid generateIndiciesToGetToVoxelImpl size {}", size);
        }

        if (size == 2)
        {
            // util::assertFatal(
            //     position.x == 0 || position.x == -1,
            //     "Invalid position.x {}",
            //     position.x);

            // util::assertFatal(
            //     position.y == 0 || position.y == -1,
            //     "Invalid position.y {}",
            //     position.y);

            // util::assertFatal(
            //     position.z == 0 || position.z == -1,
            //     "Invalid position.z {}",
            //     position.z);

            indicies.push_back(
                static_cast<std::size_t>(getOctantFromPosition(position)));
            return;
        }

        Octant o = getOctantFromPosition(position);

        indicies.push_back(static_cast<std::size_t>(o));

        Position offset =
            getOffsetPositionByOctant(o, static_cast<std::int32_t>(size / 2));

        generateIndiciesToGetToVoxelImpl(indicies, position - offset, size / 2);
    }

    // TODO: make this function not redundant
    std::vector<std::size_t>
    generateIndiciesToGetToVoxel(Position position, std::size_t totalSize)
    {
        std::vector<std::size_t> indices {};

        generateIndiciesToGetToVoxelImpl(indices, position, totalSize);

        util::logTrace("generated indices {}", util::toString(indices));

        return indices;
    }

    Voxel& VoxelOctree::access(Position globalPosition)
    {
        util::assertFatal(
            globalPosition.x >= VoxelMinimum
                && globalPosition.x <= VoxelMaximum,
            "X: {} is out of bounds!",
            globalPosition.x);
        util::assertFatal(
            globalPosition.y >= VoxelMinimum
                && globalPosition.y <= VoxelMaximum,
            "Y: {} is out of bounds!",
            globalPosition.y);
        util::assertFatal(
            globalPosition.z >= VoxelMinimum
                && globalPosition.z <= VoxelMaximum,
            "Z: {} is out of bounds!",
            globalPosition.z);

        /// Stages of the function
        /// get the traversal indicies required from node to node to reach the
        /// VoxelVolume which contians the position

        /// traverse down the tree, if at any point we encounter a nullptr we
        /// know that node isnt populated: populate it

        /// Once we reach the node retreve the reference to it

        // *pain*
        auto roundDownToNearestMultipleOfN =
            []<class I>(I multiple, I number) -> I
        {
            if (number >= 0)
            {
                return (number / multiple) * multiple;
            }
            else
            {
                return ((number - multiple + 1) / multiple) * multiple;
            }
        };

        Node* workingNode = &this->parent;

        std::size_t iteration = 0;
        for (std::size_t index :
             generateIndiciesToGetToVoxel(globalPosition, VolumeExtent))
        {
            util::logTrace("Traversal step {}", index);
            // on final iteration
            if (iteration == VoxelOctree::TraversalSteps - 1)
            {
                std::unique_ptr<VoxelVolume>* maybeVolume =
                    std::get_if<std::unique_ptr<VoxelVolume>>(
                        &workingNode->data);

                if (maybeVolume == nullptr)
                {
                    // initalize node with correct position
                    workingNode->data = std::make_unique<VoxelVolume>(Position {
                        roundDownToNearestMultipleOfN(
                            static_cast<std::int32_t>(VoxelVolume::Extent),
                            globalPosition.x),
                        roundDownToNearestMultipleOfN(
                            static_cast<std::int32_t>(VoxelVolume::Extent),
                            globalPosition.y),
                        roundDownToNearestMultipleOfN(
                            static_cast<std::int32_t>(VoxelVolume::Extent),
                            globalPosition.z),
                    });
                }
                // else just fall through and exit the loop
                // the following code already knows its a tree

                break;
            }

            std::visit(
                util::VariantHelper {
                    [&](std::unique_ptr<VoxelVolume>& volume)
                    {
                        util::panic(
                            "Found voxel volume too early! Global position: {} "
                            "| Iteration: {}",
                            static_cast<std::string>(globalPosition),
                            iteration);
                    },
                    [&](std::array<std::unique_ptr<Node>, 8>& nodes)
                    {
                        std::unique_ptr<Node>& maybeNodeToTraverse =
                            nodes[index];

                        if (maybeNodeToTraverse == nullptr)
                        {
                            maybeNodeToTraverse = std::make_unique<Node>();
                        }

                        workingNode = maybeNodeToTraverse.get();
                    }},
                workingNode->data);

            ++iteration;
        }

        // We have traversed down the tree, workingNode is now pointing towards
        // the node that contains the position that we want.
        return std::get<std::unique_ptr<VoxelVolume>>(workingNode->data)
            ->accessFromGlobalPosition(globalPosition);
    }

} // namespace game::world
