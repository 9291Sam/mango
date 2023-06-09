#include "disk_entity.hpp"
#include "util/log.hpp"
#include <gfx/renderer.hpp>
#include <unordered_map>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#pragma clang diagnostic pop

namespace game::entity
{
    DiskEntity::DiskEntity(gfx::Renderer& renderer_, const char* filepath)
        : Entity {renderer_}
        , object {nullptr}
    {
        tinyobj::attrib_t                attribute {};
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;
        std::string                      warn;
        std::string                      error;

        util::assertFatal(
            tinyobj::LoadObj(
                &attribute, &shapes, &materials, &warn, &error, filepath),
            "Failed to load file {} | Warn: {} | Error: {}",
            filepath,
            warn,
            error);

        std::vector<gfx::vulkan::Vertex> vertices;
        std::vector<gfx::vulkan::Index>  indices;

        std::unordered_map<gfx::vulkan::Vertex, std::size_t> uniqueVertices;

        for (const tinyobj::shape_t& shape : shapes)
        {
            for (const tinyobj::index_t& index : shape.mesh.indices)
            {
                const std::size_t vertexIndex =
                    static_cast<std::size_t>(index.vertex_index);
                const std::size_t normalIndex =
                    static_cast<std::size_t>(index.normal_index);
                const std::size_t texcoordIndex =
                    static_cast<std::size_t>(index.texcoord_index);

                gfx::vulkan::Vertex vertex {
                    .position {
                        index.vertex_index >= 0 ?
                        glm::vec3 {
                            attribute.vertices.at(3 * vertexIndex + 0),
                            attribute.vertices.at(3 * vertexIndex + 1),
                            attribute.vertices.at(3 * vertexIndex + 2),
                        } :
                        glm::vec3 {0.0f, 0.0f, 0.0f}
                    },
                    .color {
                        index.vertex_index >= 0 ? 
                        glm::vec4 {
                            attribute.colors.at(3 * vertexIndex + 0),
                            attribute.colors.at(3 * vertexIndex + 1),
                            attribute.colors.at(3 * vertexIndex + 2),
                            1.0f
                        } :
                        glm::vec4 {1.0f, 1.0f, 1.0f, 1.0f}
                    },
                    .normal {
                        index.normal_index >= 0 ?
                        glm::vec3 {
                            attribute.normals.at(3 * normalIndex + 0),
                            attribute.normals.at(3 * normalIndex + 1),
                            attribute.normals.at(3 * normalIndex + 2),
                        } :
                        glm::vec3 {0.0f, 0.0f, 0.0f}
                    },
                    .uv {
                        index.texcoord_index >= 0 ?
                        glm::vec2 {
                            attribute.texcoords.at(2 * texcoordIndex + 0),
                            attribute.texcoords.at(2 * texcoordIndex + 1),
                        } :
                        glm::vec2 {0.0f, 0.0f}
                    }
                };

                // If this vertex is being encountered for the first
                // time.
                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = vertices.size();

                    vertices.push_back(vertex);
                }

                util::assertWarn(
                    vertices.size() < std::numeric_limits<std::uint32_t>::max(),
                    "Tried to load a model with too many vertices!");

                indices.push_back(
                    static_cast<std::uint32_t>(uniqueVertices[vertex]));
            }
        }

        this->object = std::make_unique<gfx::SimpleTriangulatedObject>(
            this->renderer, vertices, indices);
    }

    DiskEntity::~DiskEntity() {} // NOLINT pre declarations

    void DiskEntity::tick() {}

    std::vector<const gfx::Object*> DiskEntity::draw() const
    {
        return {dynamic_cast<const gfx::Object*>(this->object.get())};
    }
} // namespace game::entity
