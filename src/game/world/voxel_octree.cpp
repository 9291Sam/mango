#include "voxel_octree.hpp"
#include "voxel.hpp"
#include <array>
#include <gfx/renderer.hpp>
#include <gfx/vulkan/includes.hpp>
#include <util/misc.hpp>
#include <utility>
#include <vector>

namespace game::world
{
    class Node;

    class Node
    {
    public:

        Node()
            : children {Voxel {glm::vec4 {0.0f, 0.5f, 0.6f, 1.0f}}}
        {}
        ~Node() = default;

        Node(const Node&)             = delete;
        Node(Node&&)                  = default;
        Node& operator= (const Node&) = delete;
        Node& operator= (Node&&)      = default;

        std::variant<Voxel, std::array<std::unique_ptr<Node>, 8>> children;
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
        float                             size)
    {
        const std::array<gfx::vulkan::Vertex, 8> cube_vertices {
            gfx::vulkan::Vertex {
                .position {-1.0f, -1.0f, -1.0f},
                .color {0.0f, 0.0f, 0.0f, 1.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-1.0f, -1.0f, 1.0f},
                .color {0.0f, 0.0f, 1.0f, 1.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-1.0f, 1.0f, -1.0f},
                .color {0.0f, 1.0f, 0.0f, 1.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-1.0f, 1.0f, 1.0f},
                .color {0.0f, 1.0f, 1.0f, 1.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, -1.0f, -1.0f},
                .color {1.0f, 0.0f, 0.0f, 1.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, -1.0f, 1.0f},
                .color {1.0f, 0.0f, 1.0f, 1.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, 1.0f, -1.0f},
                .color {1.0f, 1.0f, 0.0f, 1.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, 1.0f, 1.0f},
                .color {1.0f, 1.0f, 1.0f, 1.0f},
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
                    generateTrianglesFromVoxel(vertices, voxelPosition, size);
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
        , center_position {0.0f, 8.75f, 0.0f}
        , total_dimension {1.25f}
        , number_of_voxels {1}
        , voxels_per_object {10'000} // TODO: tune
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
                this->total_dimension,
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
} // namespace game::world
