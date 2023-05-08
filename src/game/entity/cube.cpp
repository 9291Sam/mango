#include "cube.hpp"
#include <gfx/object.hpp>
#include <gfx/renderer.hpp>
#include <gfx/vulkan/gpu_data.hpp>

namespace game::entity
{
    Cube::Cube(std::shared_ptr<gfx::Renderer> renderer_)
        : Entity {std::move(renderer_)}
    {
        const std::array<gfx::vulkan::Vertex, 8> vertices {
            gfx::vulkan::Vertex {
                .position {-1.0f, -1.0f, -1.0f},
                .color {0.0f, 0.0f, 0.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-1.0f, -1.0f, 1.0f},
                .color {0.0f, 0.0f, 1.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-1.0f, 1.0f, -1.0f},
                .color {0.0f, 1.0f, 0.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {-1.0f, 1.0f, 1.0f},
                .color {0.0f, 1.0f, 1.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, -1.0f, -1.0f},
                .color {1.0f, 0.0f, 0.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, -1.0f, 1.0f},
                .color {1.0f, 0.0f, 1.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, 1.0f, -1.0f},
                .color {1.0f, 1.0f, 0.0f},
                .normal {},
                .uv {},
            },
            gfx::vulkan::Vertex {
                .position {1.0f, 1.0f, 1.0f},
                .color {1.0f, 1.0f, 1.0f},
                .normal {},
                .uv {},
            },
        };

        const std::array<gfx::vulkan::Index, 36> indices {
            2, 6, 7, 2, 3, 7, 0, 4, 5, 0, 1, 5, 0, 2, 6, 0, 4, 6,
            1, 3, 7, 1, 5, 7, 0, 2, 3, 0, 1, 3, 4, 6, 7, 4, 5, 7};

        this->object = this->renderer->createObject(
            gfx::Renderer::PipelineType::FlatPipeline, vertices, indices);

        this->object.transform.translation.y += 12.5f;
    }

    void Cube::tick(float deltaTime)
    {
        this->object.transform.yawBy(1.0f * deltaTime);
    }

    std::vector<const gfx::Object*> Cube::lend() const
    {
        return {&this->object};
    }
} // namespace game::entity
