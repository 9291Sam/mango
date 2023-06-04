#include "world.hpp"
#include "game/world/voxel_octree.hpp"
#include "gfx/renderer.hpp"
#include "gfx/vulkan/pipelines.hpp"
#include <numbers>

namespace game::world
{
    World::World(gfx::Renderer& renderer_)
        : renderer {renderer_}
    {
        for (std::int32_t ox : std::views::iota(
                 VoxelOctree::VoxelMinimum, VoxelOctree::VoxelMaximum))
        {
            for (std::int32_t oy : std::views::iota(
                     VoxelOctree::VoxelMinimum, VoxelOctree::VoxelMaximum))
            {
                const float normalizedX =
                    static_cast<float>(ox)
                    / static_cast<float>(VoxelOctree::VolumeExtent);
                const float normalizedY =
                    static_cast<float>(oy)
                    / static_cast<float>(VoxelOctree::VolumeExtent);

                const float pi4  = static_cast<float>(std::numbers::pi) * 2;
                const float sinX = std::sin(normalizedX * pi4);
                const float cosY = std::cos(normalizedY * pi4);

                // std::int32_t height = static_cast<std::int32_t>(
                //     std::atan2(1, 75 * normalizedX * normalizedY) * 25); //
                //     250

                // height += static_cast<std::int32_t>(8 * sinX + 8 * cosY);

                // std::int32_t height = (ox + oy) / 2;
                std::int32_t height = 0;

                float color = util::map(sinX + cosY, -2.83f, 2.83f, 0.0f, 1.0f);

                color = std::sin(color * 100.0f);

                // TODO: make flat and detect multiple accesses to the same
                // place?
                this->octree.access(world::Position {
                    ox, height, oy}) = world::Voxel {glm::vec4 {
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