#include "cube.hpp"
#include <gfx/object.hpp>
#include <gfx/renderer.hpp>
#include <gfx/vulkan/gpu_data.hpp>

namespace game::entity
{
    Cube::Cube(
        std::shared_ptr<gfx::Renderer> renderer_, gfx::Transform transform)
        : Entity {std::move(renderer_)}
        , object {[this]
                  {
                      const std::array<gfx::vulkan::Vertex, 8> vertices {
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

                      return this->renderer->createTriangulatedObject(
                          gfx::vulkan::PipelineType::Flat, vertices, indices);
                  }()}
    {
        this->object.transform = transform;
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
