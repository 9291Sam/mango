#include "voxel_octree.hpp"
#include "game/world/voxel_octree.hpp"
#include "gfx/vulkan/gpu_data.hpp"
#include "util/misc.hpp"
#include <functional>
#include <ranges>
#include <util/log.hpp>
#include <variant>

namespace game::world
{
    Position getOffsetPositionByOctant(Octant octant, std::int32_t size)
    {
        using enum Octant;

        switch (octant)
        {
        case pXpYpZ:
            return Position {size, size, size};
        case pXpYnZ:
            return Position {size, size, -size};
        case pXnYpZ:
            return Position {size, -size, size};
        case pXnYnZ:
            return Position {size, -size, -size};
        case nXpYpZ:
            return Position {-size, size, size};
        case nXpYnZ:
            return Position {-size, size, -size};
        case nXnYpZ:
            return Position {-size, -size, size};
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

    Position Position::operator/ (std::int32_t number) const
    {
        return Position {
            this->x / number,
            this->y / number,
            this->z / number,
        };
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
        // util::logTrace(
        //     "Doing global lookup: {}",
        //     static_cast<std::string>(globalPosition));
        return this->accessFromLocalPosition(
            globalPosition - this->local_offset);
    }

    Voxel& VoxelVolume::accessFromLocalPosition(Position localPosition)
    {
        // util::logTrace(
        //     "Doing local lookup: {}",
        //     static_cast<std::string>(localPosition));

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

                    if (!voxel.shouldDraw())
                    {
                        continue;
                    }

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

                        v.position += static_cast<glm::vec3>(
                            Position {localX, localY, localZ});

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

    std::array<std::size_t, VoxelOctree::TraversalSteps>
    generateIndiciesToGetToVoxelVolume(Position position)
    {
        std::size_t octantSize = VoxelOctree::VolumeExtent / 2;

        std::vector<std::size_t> outputvec;

        while (octantSize != 0)
        {
            const Position origionalPosition = position;
            const Octant   octant            = getOctantFromPosition(position);
            const Position offsetPosition =
                getOffsetPositionByOctant(octant, octantSize / 2);

            outputvec.push_back(
                static_cast<std::size_t>(util::toUnderlyingType(octant)));

            position = position - offsetPosition;

            // position = position / 2;

            // util::logTrace(
            //     "Iteration! | OctantSize: {} | OPosition: {} | MPosition: {}
            //     | " "octant: {} | offset: {}", octantSize,
            //     static_cast<std::string>(origionalPosition),
            //     static_cast<std::string>(position),
            //     util::toUnderlyingType(octant),
            //     static_cast<std::string>(offsetPosition));

            octantSize /= 2;
        }

        // util::logTrace("Generated indices {}", util::toString(outputvec));

        std::array<std::size_t, VoxelOctree::TraversalSteps> trunc {};

        for (std::size_t i = 0; i < trunc.size(); ++i)
        {
            trunc[i] = outputvec.at(i);
        }

        // util::logTrace(
        //     "Generated Indices {} |T {}",
        //     util::toString(outputvec),
        //     util::toString(trunc));

        return trunc;
    }

    Voxel& VoxelOctree::access(Position globalPosition)
    {
        // util::logTrace("\n\nStarted new access!");
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

        // std::ignore = generateIndiciesToGetToVoxelVolume(Position {1602, 1,
        // 0}); util::panic("Earlyterm");

        Node* workingNode = &this->parent;

        for (std::size_t index :
             generateIndiciesToGetToVoxelVolume(globalPosition))
        {
            // util::logTrace("Traversal step {}", index);

            std::visit(
                util::VariantHelper {
                    [&](std::unique_ptr<VoxelVolume>& volume)
                    {
                        util::panic(
                            "Found voxel volume too early! Global position: {} "
                            "| Index: {}",
                            static_cast<std::string>(globalPosition),
                            index);
                    },
                    [&](std::array<std::unique_ptr<Node>, 8>& nodes)
                    {
                        std::unique_ptr<Node>& maybeNodeToTraverse =
                            nodes[index];

                        if (maybeNodeToTraverse == nullptr)
                        {
                            maybeNodeToTraverse = std::make_unique<Node>();
                        }

                        // util::logTrace(
                        //     "Traversing to node {} @ {}",
                        //     (void*)(maybeNodeToTraverse.get()),
                        //     index);
                        workingNode = maybeNodeToTraverse.get();
                    }},
                workingNode->data);

            // on final iteration
            // if (iteration == VoxelOctree::TraversalSteps - 1)
            // {
            //     util::logTrace("Broke out of traversal!");
            //     break;
            // }

            // ++iteration;
        }

        // We now have the last node. It's either a volume in which case
        // everything is good. Or its more nodes in which case we need to
        // replace it with a volume
        if (std::array<std::unique_ptr<Node>, 8>* ptr =
                std::get_if<std::array<std::unique_ptr<Node>, 8>>(
                    &workingNode->data))
        {
            // The last node isn't a VoxelVolume, make it one
            for (const std::unique_ptr<Node>& n : *ptr)
            {
                util::assertFatal(
                    n == nullptr, "Node traversal was not nullptr");
            }

            Position volumePosition {
                roundDownToNearestMultipleOfN(
                    static_cast<std::int32_t>(VoxelVolume::Extent),
                    globalPosition.x),
                roundDownToNearestMultipleOfN(
                    static_cast<std::int32_t>(VoxelVolume::Extent),
                    globalPosition.y),
                roundDownToNearestMultipleOfN(
                    static_cast<std::int32_t>(VoxelVolume::Extent),
                    globalPosition.z),
            };

            workingNode->data = std::make_unique<VoxelVolume>(volumePosition);

            // util::logTrace(
            //     "Instantiated new Volume @ {} | {}",
            //     (void*)(std::get_if<std::unique_ptr<VoxelVolume>>(
            //         &workingNode->data)),
            //     static_cast<std::string>(volumePosition));
        }

        // util::logTrace(
        //     "Working volulume Addr: {}",
        //     (void*)(&std::get<std::unique_ptr<VoxelVolume>>(
        //         workingNode->data)));

        // We have traversed down the tree, workingNode is now pointing
        // towards the node that contains the position that we want.
        return std::get<std::unique_ptr<VoxelVolume>>(workingNode->data)
            ->accessFromGlobalPosition(globalPosition);
    }

} // namespace game::world
