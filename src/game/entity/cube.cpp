#include "cube.hpp"
#include <gfx/object.hpp>
#include <gfx/renderer.hpp>
#include <gfx/vulkan/gpu_data.hpp>

static constexpr std::array<gfx::vulkan::Vertex, 8> vertices {
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

static constexpr std::array<gfx::vulkan::Index, 36> indices {
    6, 2, 7, 2, 3, 7, 0, 4, 5, 1, 0, 5, 0, 2, 6, 4, 0, 6,
    3, 1, 7, 1, 5, 7, 2, 0, 3, 0, 1, 3, 4, 6, 7, 5, 4, 7};

game::entity::Cube::Cube(gfx::Renderer& renderer_, glm::vec3 position)
    : Entity {renderer_}
    , object {this->renderer, std::span {vertices}, std::span {indices}}
{
    this->object.transform.translation = position;
}

void game::entity::Cube::tick()
{
    this->object.transform.yawBy(1.0f * this->renderer.getDeltaTimeSeconds());
}

std::vector<const gfx::Object*> game::entity::Cube::draw() const
{
    return {dynamic_cast<const gfx::Object*>(&this->object)};
}
