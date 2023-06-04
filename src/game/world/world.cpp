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
                const float normalizedX = util::map<float>(
                    static_cast<float>(ox),
                    static_cast<float>(VoxelOctree::VoxelMinimum),
                    static_cast<float>(VoxelOctree::VoxelMaximum),
                    -1.0f,
                    1.0f);

                const float normalizedY = util::map<float>(
                    static_cast<float>(oy),
                    static_cast<float>(VoxelOctree::VoxelMinimum),
                    static_cast<float>(VoxelOctree::VoxelMaximum),
                    -1.0f,
                    1.0f);

                auto height = [=]() -> std::int32_t
                {
                    const float rad =
                        static_cast<float>(std::numbers::pi) * 120;

                    std::int32_t height =
                        std::atan2(
                            1.0f, std::numbers::pi * normalizedX * normalizedY)
                        * 256;

                    height += std::sin(normalizedX * rad) * 3;
                    height += std::cos(normalizedY * rad) * 3;

                    return height - 450;
                };

                world::Position outputPosition {ox, height(), oy};

                glm::vec4 color {
                    std::abs(
                        normalizedX * normalizedY * normalizedX * normalizedY),
                    util::map(normalizedX, -1.0f, 1.0f, 0.0f, 0.7f),
                    util::map(normalizedY, -1.0f, 1.0f, 0.0f, 0.7f),
                    1.0f};

                if (height() % 2 == 0)
                {
                    color.r = 0.125f;
                }

                color /= 1.35f;
                color.a = 1.0f;

                this->octree.access(outputPosition) = world::Voxel {color};
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