#include "voxel_octree.hpp"
#include "voxel.hpp"
#include <array>
#include <gfx/renderer.hpp>
#include <gfx/vulkan/includes.hpp>
#include <iostream>
#include <util/misc.hpp>
#include <utility>
#include <vector>

// TODO: lmfao this file is a fucking mess!

namespace game::world
{
    class Node;

    class Node
    {
    public:

        Node(Voxel v)
            : children {v}
        {}
        Node(std::array<std::unique_ptr<Node>, 8> nodes)
            : children {std::move(nodes)}
        {}
        Node()
            : children {Voxel {glm::vec4 {0.75f, 0.5f, 0.6f, 0.1f}}}
        {}
        ~Node() = default;

        Node(const Node&)             = delete;
        Node(Node&&)                  = default;
        Node& operator= (const Node&) = delete;
        Node& operator= (Node&&)      = default;

        std::variant<Voxel, std::array<std::unique_ptr<Node>, 8>> children;
    };

    float getTotalSizeFromLevels(std::size_t levels)
    {
        switch (levels)
        {
        case 0:
            [[unlikely]];
            util::panic("Invalid level size {}", levels);
            std::unreachable();
        default:
            [[likely]];
            return util::exponentiate(2UZ, levels - 1);
        }
    }

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

    glm::vec3 getOffsetPositionByOctant(Octant octant, float size)
    {
        using enum Octant;

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

        util::panic(
            "Unreachable enum {}", static_cast<std::uint_fast8_t>(octant));
        std::unreachable();
    }

    std::array<std::pair<Node*, Octant>, 8>
    iterateOverChildren(std::array<std::unique_ptr<Node>, 8>& children)
    {
        return std::array<std::pair<Node*, Octant>, 8> {
            std::make_pair(children[0].get(), static_cast<Octant>(0)),
            std::make_pair(children[1].get(), static_cast<Octant>(1)),
            std::make_pair(children[2].get(), static_cast<Octant>(2)),
            std::make_pair(children[3].get(), static_cast<Octant>(3)),
            std::make_pair(children[4].get(), static_cast<Octant>(4)),
            std::make_pair(children[5].get(), static_cast<Octant>(5)),
            std::make_pair(children[6].get(), static_cast<Octant>(6)),
            std::make_pair(children[7].get(), static_cast<Octant>(7))};
    }

    void generateTrianglesFromVoxel(
        std::vector<gfx::vulkan::Vertex>& vertices,
        glm::vec3                         position,
        Voxel                             voxel,
        float                             size)
    {
        if (voxel.linear_color.a == 0.0f)
        {
            return;
        }

        const std::array<gfx::vulkan::Vertex, 8> cube_vertices {
            gfx::vulkan::Vertex {
                .position {-1.0f, -1.0f, -1.0f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-1.0f, -1.0f, 1.0f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-1.0f, 1.0f, -1.0f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-1.0f, 1.0f, 1.0f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, -1.0f, -1.0f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, -1.0f, 1.0f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, 1.0f, -1.0f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, 1.0f, 1.0f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
        };

        const std::array<gfx::vulkan::Index, 36> indices {
            6, 2, 7, 2, 3, 7, 0, 4, 5, 1, 0, 5, 0, 2, 6, 4, 0, 6,
            3, 1, 7, 1, 5, 7, 2, 0, 3, 0, 1, 3, 4, 6, 7, 5, 4, 7};

        for (gfx::vulkan::Index idx : indices)
        {
            gfx::vulkan::Vertex vertex = cube_vertices[idx];

            vertex.position *= size;
            vertex.position += position;

            vertices.push_back(vertex);
        }
    }

    void generateVerticesFromOctree(
        float                             size,
        glm::vec3                         voxelPosition,
        std::vector<gfx::vulkan::Vertex>& vertices,
        Node*                             node)
    {
        if (node == nullptr)
        {
            util::logWarn("node base case!");
            return;
        }

        std::visit(
            util::VariantHelper {
                [&](Voxel v)
                {
                    generateTrianglesFromVoxel(
                        vertices, voxelPosition, v, size);
                },
                [&](std::array<std::unique_ptr<Node>, 8>& children)
                {
                    for (auto& [node, octant] : iterateOverChildren(children))
                    {
                        glm::vec3 newPosition =
                            voxelPosition
                            + getOffsetPositionByOctant(octant, size / 2);

                        generateVerticesFromOctree(
                            size / 2, newPosition, vertices, node);
                    }
                }},
            node->children);
    }

    VoxelOctree::VoxelOctree(gfx::Renderer& renderer_)
        : renderer {renderer_}
        , objects {}
        , root {std::make_unique<Node>()}
        , center_position {32.0f, 32.0f, 32.0f}
        , levels {5}
        , dimension {util::exponentiate<std::size_t>(2, this->levels)}
    // , number_of_voxels {1}
    // , voxels_per_object {10'000} // TODO: tune
    {}

    VoxelOctree::~VoxelOctree() {}

    std::vector<std::shared_ptr<gfx::Object>>
    VoxelOctree::draw([[maybe_unused]] glm::vec3 lookAtVector)
    {
        if (this->objects.size() == 0)
        {
            std::vector<gfx::vulkan::Vertex> vertices {};
            std::vector<gfx::vulkan::Index>  indices {};

            generateVerticesFromOctree(
                static_cast<float>(this->dimension),
                this->center_position,
                vertices,
                this->root.get());

            indices.resize(vertices.size());

            std::iota(indices.begin(), indices.end(), 0);

            // TODO: I just got bugged by something that would have been caught
            // by nodiscard...
            this->objects.push_back(this->renderer.createTriangulatedObject(
                gfx::vulkan::PipelineType::Flat, vertices, indices));

            util::logTrace(
                "Created world with sizes of {} {}",
                vertices.size(),
                indices.size());
        }

        return this->objects; // copy!
    }

    Octant getOctantFromPosition(LocalPosition position)
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
        std::unreachable();
    }

    LocalPosition getOctantCenter(Octant o, std::size_t size)
    {
        glm::vec3 multipliers = getOffsetPositionByOctant(o, 1.0f);

        std::int32_t xMultiplier = static_cast<std::int32_t>(multipliers.x);
        std::int32_t yMultiplier = static_cast<std::int32_t>(multipliers.y);
        std::int32_t zMultiplier = static_cast<std::int32_t>(multipliers.z);

        return LocalPosition {
            .x {xMultiplier * static_cast<std::int32_t>(size / 2)},
            .y {yMultiplier * static_cast<std::int32_t>(size / 2)},
            .z {zMultiplier * static_cast<std::int32_t>(size / 2)}};
    }

    bool isOneZeroOrNegativeOne(std::int32_t i)
    {
        if (i == 0 || i == -1 || i == 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void generateImpl(
        std::vector<std::size_t>& indices,
        LocalPosition             position,
        std::size_t               size)
    {
        Octant o = getOctantFromPosition(position);

        indices.push_back(static_cast<std::size_t>(o));

        LocalPosition octantCenter = getOctantCenter(o, size / 2);

        LocalPosition offsetPosition {
            .x {position.x - octantCenter.x},
            .y {position.y - octantCenter.y},
            .z {position.z - octantCenter.z}};

        if (isOneZeroOrNegativeOne(offsetPosition.x)
            && isOneZeroOrNegativeOne(offsetPosition.y)
            && isOneZeroOrNegativeOne(offsetPosition.z))
        {
            return;
        }

        generateImpl(indices, offsetPosition, size / 2);
    }

    std::vector<std::size_t>
    generateIndiciesToGetToVoxel(LocalPosition position, std::size_t totalSize)
    {
        std::vector<std::size_t> indices {};

        generateImpl(indices, position, totalSize);

        return indices;
    }

    void VoxelOctree::insertVoxelAtPosition(Voxel voxel, LocalPosition position)
    {
        util::assertFatal(
            std::abs(position.x) <= this->dimension / 2,
            "X position {0} is outside tree of dimension -{1} -> +{1}",
            position.x,
            this->dimension / 2);

        util::assertFatal(
            std::abs(position.y) <= this->dimension / 2,
            "Y position {0} is outside tree of dimension -{1} -> +{1}",
            position.y,
            this->dimension / 2);

        util::assertFatal(
            std::abs(position.z) <= this->dimension / 2,
            "Z position {0} is outside tree of dimension -{1} -> +{1}",
            position.z,
            this->dimension / 2);

        std::vector<std::size_t> indicesToVoxel =
            generateIndiciesToGetToVoxel(position, this->dimension);

        Node* workingNode = this->root.get();

        for (std::size_t index : indicesToVoxel)
        {
            if (std::holds_alternative<Voxel>(workingNode->children))
            {
                // populate required tree nodes

                Voxel voxelToPopulate =
                    *std::get_if<Voxel>(&workingNode->children);

                std::array<std::unique_ptr<Node>, 8> array {nullptr};
                for (std::unique_ptr<Node>& n : array)
                {
                    n = std::make_unique<Node>(voxelToPopulate);
                }

                workingNode->children =
                    std::variant<Voxel, std::array<std::unique_ptr<Node>, 8>> {
                        std::move(array)};
            }

            workingNode = (*std::get_if<std::array<std::unique_ptr<Node>, 8>>(
                &workingNode->children))[index]
                              .get();
        }

        // workingNode is now a pointer to the node that we want
        workingNode->children =
            std::variant<Voxel, std::array<std::unique_ptr<Node>, 8>> {voxel};

        this->objects.clear(); // reset
    }

} // namespace game::world
