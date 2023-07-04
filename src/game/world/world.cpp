
#include "game/world/world.hpp"
#include "gfx/object.hpp"
#include "gfx/renderer.hpp"
#include "gfx/vulkan/gpu_data.hpp"
#include <memory>

game::world::World::World(gfx::Renderer& renderer_)
    : renderer {renderer_}
{
    constexpr std::array<gfx::vulkan::Vertex, 4> vertices {
        gfx::vulkan::Vertex {
            .position {10.0f, 0.0f, 10.0f},
            .color {1.0f, 1.0f, 1.0f, 1.0f},
            .normal {1.0f, 0.0f, 0.0f},
            .uv {1.0f, 1.0f}},
        gfx::vulkan::Vertex {
            .position {-10.0f, 0.0f, 10.0f},
            .color {1.0f, 1.0f, 1.0f, 1.0f},
            .normal {1.0f, 0.0f, 0.0f},
            .uv {1.0f, 1.0f}},
        gfx::vulkan::Vertex {
            .position {10.0f, 0.0f, -10.0f},
            .color {1.0f, 1.0f, 1.0f, 1.0f},
            .normal {1.0f, 0.0f, 0.0f},
            .uv {1.0f, 1.0f}},
        gfx::vulkan::Vertex {
            .position {-10.0f, 0.0f, -10.0f},
            .color {1.0f, 1.0f, 1.0f, 1.0f},
            .normal {1.0f, 0.0f, 0.0f},
            .uv {1.0f, 1.0f}},
    };

    constexpr std::array<gfx::vulkan::Index, 6> indices {0, 1, 2, 1, 2, 3};

    this->objects.push_back(std::make_shared<gfx::SimpleTriangulatedObject>(
        this->renderer, vertices, indices));
}

std::vector<std::shared_ptr<gfx::Object>> game::world::World::draw() const
{
    return this->objects;
}
