#include "world.hpp"
#include "gfx/renderer.hpp"
#include "gfx/vulkan/pipelines.hpp"
#include <numbers>

namespace game::world
{
    World::World(gfx::Renderer& renderer_)
        : renderer {renderer_}
    {
        const std::size_t worldDimension =
            world::VoxelOctree::Extent * world::VoxelOctree::VoxelVolume::Size;
        const std::int32_t extent =
            static_cast<std::int32_t>(worldDimension / 2);

        for (std::int32_t x : std::views::iota(-extent, extent))
        {
            for (std::int32_t y : std::views::iota(-extent, extent))
            {
                const float normalizedX =
                    static_cast<float>(x) / static_cast<float>(worldDimension);
                const float normalizedY =
                    static_cast<float>(y) / static_cast<float>(worldDimension);

                const float pi4  = static_cast<float>(std::numbers::pi) * 80;
                const float sinX = std::sin(normalizedX * pi4);
                const float cosY = std::cos(normalizedY * pi4);

                std::int32_t height = static_cast<std::int32_t>(
                    std::atan2(1, 75 * normalizedX * normalizedY) * 256); // 250

                height += static_cast<std::int32_t>(8 * sinX + 8 * cosY);

                float color = util::map(sinX + cosY, -2.83f, 2.83f, 0.0f, 1.0f);

                color = std::sin(color * 100.0f);

                this->octree.access(world::VoxelOctree::Position {
                    x, height, y}) = world::VoxelOctree::Voxel {glm::vec4 {
                    color, std::abs(normalizedX), std::abs(normalizedY), 1.0f}};
            }
        }

        auto [vertices, indices] = this->octree.draw();

        this->objects.push_back(this->renderer.createTriangulatedObject(
            gfx::vulkan::PipelineType::Flat,
            std::move(vertices),
            std::move(indices)));

        util::logTrace("World initalization complete");
    }

    std::vector<std::shared_ptr<gfx::Object>> World::draw() const
    {
        return this->objects;
    }
} // namespace game::world