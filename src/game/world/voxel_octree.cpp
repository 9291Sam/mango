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

    LocalPosition::operator std::string ()
    {
        return fmt::format(
            "LocalPosition X: {} | Y: {} | Z: {}", this->x, this->y, this->z);
    }
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
            : children {Voxel {glm::vec4 {0.0f, 0.0f, 0.0f, 0.0f}}}
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

    LocalPosition getOffsetPositionByOctant(Octant octant, std::int32_t size)
    {
        using enum Octant;

        switch (octant)
        {
        case pXpYpZ:
            return LocalPosition {.x {size}, .y {size}, .z {size}};

        case pXpYnZ:
            return LocalPosition {.x {size}, .y {size}, .z {-size}};

        case pXnYpZ:
            return LocalPosition {.x {size}, .y {-size}, .z {size}};

        case pXnYnZ:
            return LocalPosition {.x {size}, .y {-size}, .z {-size}};

        case nXpYpZ:
            return LocalPosition {.x {-size}, .y {size}, .z {size}};

        case nXpYnZ:
            return LocalPosition {.x {-size}, .y {size}, .z {-size}};

        case nXnYpZ:
            return LocalPosition {.x {-size}, .y {-size}, .z {size}};

        case nXnYnZ:
            return LocalPosition {.x {-size}, .y {-size}, .z {-size}};
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
        if (!voxel.shouldDraw())
        {
            return;
        }

        const std::array<gfx::vulkan::Vertex, 8> cube_vertices {
            gfx::vulkan::Vertex {
                .position {-0.5f, -0.5f, -0.5f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-0.5f, -0.5f, 0.5f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-0.5f, 0.5f, -0.5f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-0.5f, 0.5f, 0.5f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {0.5f, -0.5f, -0.5f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {0.5f, -0.5f, 0.5f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {0.5f, 0.5f, -0.5f},
                .color {voxel.linear_color},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {0.5f, 0.5f, 0.5f},
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
        std::size_t                       size,
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
                        vertices, voxelPosition, v, static_cast<float>(size));
                },
                [&](std::array<std::unique_ptr<Node>, 8>& children)
                {
                    if (size == 1)
                    {
                        util::panic(
                            "Tried to access child nodes of node of size 1");
                    }
                    else
                    {
                        // TODO: print out the required indicies to traverse
                        // down wqhere veer taht is

                        for (auto& [node, octant] :
                             iterateOverChildren(children))
                        {
                            generateVerticesFromOctree(
                                size / 2,
                                voxelPosition
                                    + getOffsetPositionByOctant(
                                        octant, static_cast<float>(size) / 4),
                                vertices,
                                node);
                        }
                    }
                }},
            node->children);
    }

    VoxelOctree::VoxelOctree(
        gfx::Renderer& renderer_, glm::vec3 centerPosition, std::size_t levels_)
        : renderer {renderer_}
        , objects {}
        , root {std::make_unique<Node>()}
        , center_position {centerPosition}
        , levels {levels_}
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
                this->dimension,
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

    void generateIndiciesToGetToVoxelImpl(
        std::vector<std::size_t>& indicies,
        LocalPosition             position,
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

        LocalPosition offset =
            getOffsetPositionByOctant(o, static_cast<std::int32_t>(size / 2));

        generateIndiciesToGetToVoxelImpl(indicies, position - offset, size / 2);
    }

    std::vector<std::size_t>
    generateIndiciesToGetToVoxel(LocalPosition position, std::size_t totalSize)
    {
        std::vector<std::size_t> indices {};

        generateIndiciesToGetToVoxelImpl(indices, position, totalSize);

        return indices;
    }

    void VoxelOctree::insertVoxelAtPosition(Voxel voxel, LocalPosition position)
    {
        // TODO: these arent accurate since its actually like -127 -> +128 ish
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

        util::assertWarn(
            indicesToVoxel.size() == this->levels,
            "Invalid number of indicies | Levels: {} | Indicies: {} | "
            "DesiredPosition: {}",
            this->levels,
            util::stringifyVector(indicesToVoxel),
            static_cast<std::string>(position));

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