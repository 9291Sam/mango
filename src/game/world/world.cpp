#include "world.hpp"
#include <gfx/renderer.hpp>

namespace game::world
{
    World::World(
        std::shared_ptr<gfx::Renderer>              renderer_,
        [[maybe_unused]] std::optional<std::size_t> seed)
        : renderer {std::move(renderer_)} // clang-format off
        , voxels {{-0.0f, 0.0f, 32.0f},
            32,
            Voxel {
                .r {0},
                .g {255},
                .b {255},
                .a {255}
            }
        }
        // clang-format on
        , object {[this]
                  {
                      auto [vertices, indices] =
                          this->voxels.getVerticesAndIndices();

                      return this->renderer->createTriangulatedObject(
                          gfx::vulkan::PipelineType::Flat, vertices, indices);
                  }()}
        , voxel_object {[this]
                        {
                            std::array<glm::vec3, 1> positions {
                                glm::vec3 {0.0f, 32.0f, 0.0f}};

                            return this->renderer->createVoxelObject(positions);
                        }()}
        , disk_triangle {this->renderer, "../models/floor.obj"}
    {
        this->disk_triangle.object.transform.translation.y += 145.0f;

        this->disk_triangle.object.transform.rotation =
            glm::quat {1.0f, 0.0f, 0.0f, 0.0f};

        this->disk_triangle.object.transform.scale *= 50;
    }

    std::vector<const gfx::Object*> World::lend() const
    {
        return {
            &this->object, &this->voxel_object, &this->disk_triangle.object};
    }
} // namespace game::world
