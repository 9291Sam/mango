#include "world.hpp"
#include <gfx/renderer.hpp>

namespace game::world
{
    World::World(
        std::shared_ptr<gfx::Renderer> renderer_,
        std::optional<std::size_t>     seed)
        : renderer {std::move(renderer_)} // clang-format off
        , voxels {
            gfx::Transform {
                .translation {-10.0f, 10.0f, 10.0f},
                .rotation {1.0f, 0.0f, 0.0f, 0.0f},
                .scale {1.0f, 1.0f, 1.0f}
            },
            16,
            Voxel {
                .r {0},
                .g {255},
                .b {255},
                .a {255}
            }
        }
        // clang-format on
        , object {}
    {
        auto [vertices, indices] = this->voxels.getVerticesAndIndicies();

        this->object = this->renderer->createObject(
            gfx::Renderer::PipelineType::FlatPipeline, vertices, indices);
    }

    World::~World() {}

    std::vector<const gfx::Object*> World::lend() const
    {
        return {&this->object};
    }
} // namespace game::world