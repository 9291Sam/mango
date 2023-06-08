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
                    const float rad = static_cast<float>(std::numbers::pi) * 60;

                    auto sebs = [=](float scale, float exp, float bias) -> float
                    {
                        return scale
                             * std::sin(
                                   exp * bias * normalizedX
                                   + exp * scale * normalizedY + bias);
                    };

                    float height = 0.0f;

                    // std::int32_t height =
                    //     std::atan2(
                    //         1.0f, std::numbers::pi * normalizedX *
                    //         normalizedY)
                    //     * 128;

                    // height += std::sin(normalizedX * rad) * 5;
                    // height += std::cos(normalizedY * rad) * 5;

                    height += sebs(-2.6f, -21.4f, 23.3f);
                    height += sebs(-6.3f, -11.5f, 4.9f);
                    height += sebs(-23.7f, 3.6f, 20.8f);
                    height += sebs(17.7f, -3.5f, -18.7f);
                    height += sebs(-6.7f, 18.2f, 5.1f);
                    height += sebs(17.1f, 18.0f, -7.3f);
                    height += sebs(20.1f, 22.0f, 13.6f);
                    height += sebs(5.2f, -22.3f, -15.2f);
                    height += sebs(-15.0f, -20.7f, -8.4f);
                    height += sebs(-20.0f, 2.6f, 0.2f);
                    height += sebs(17.8f, 25.0f, 3.8f);
                    height += sebs(16.1f, -6.3f, 17.2f);
                    height += sebs(-4.0f, -5.0f, -4.1f);
                    height += sebs(-8.2f, -12.4f, 17.3f);
                    height += sebs(-1.9f, 6.0f, 10.5f);
                    height += sebs(-21.8f, -2.0f, 3.5f);
                    height += sebs(-4.1f, -6.9f, 0.5f);
                    height += sebs(-15.5f, 23.2f, 3.2f);
                    height += sebs(-5.7f, -16.7f, 7.5f);
                    height += sebs(10.9f, -11.7f, -14.9f);
                    height += sebs(-13.9f, -18.9f, -14.9f);
                    height += sebs(14.8f, 22.1f, 3.8f);
                    height += sebs(7.1f, -6.9f, 23.8f);
                    height += sebs(-22.1f, 9.9f, -13.6f);
                    height += sebs(20.2f, -17.1f, 17.4f);
                    height += sebs(5.8f, -13.8f, -9.6f);
                    height += sebs(-15.7f, 7.1f, 8.5f);
                    height += sebs(9.8f, 16.5f, 6.7f);
                    height += sebs(-12.0f, -14.2f, 15.0f);
                    height += sebs(20.5f, 21.6f, -21.7f);
                    height += sebs(-19.3f, -17.3f, -15.6f);
                    height += sebs(-9.2f, 22.5f, -18.5f);
                    height += sebs(19.6f, 4.6f, -0.4f);
                    height += sebs(18.3f, 7.5f, -15.5f);
                    height += sebs(-20.3f, 2.8f, -7.6f);
                    height += sebs(22.1f, -16.5f, 3.6f);
                    height += sebs(-12.5f, -7.2f, -14.4f);
                    height += sebs(21.0f, -22.4f, -22.8f);
                    height += sebs(2.7f, -9.3f, -12.0f);
                    height += sebs(-7.3f, -13.8f, -15.4f);
                    height += sebs(-4.0f, -8.1f, -7.7f);
                    height += sebs(5.4f, 7.7f, -10.1f);
                    height += sebs(11.9f, 2.3f, 19.5f);
                    height += sebs(-13.5f, -4.4f, 2.0f);
                    height += sebs(-4.1f, 0.4f, -20.3f);
                    height += sebs(13.3f, -2.5f, -11.7f);
                    height += sebs(-8.1f, 10.0f, 6.1f);
                    height += sebs(2.5f, 6.0f, 8.8f);
                    height += sebs(4.7f, -16.3f, 24.4f);
                    height += sebs(17.7f, 1.4f, -14.4f);
                    height += sebs(-22.7f, 22.4f, 2.5f);
                    height += sebs(-5.3f, 2.8f, -14.8f);
                    height += sebs(9.5f, -10.3f, 12.5f);
                    height += sebs(4.0f, 1.2f, -16.9f);
                    height += sebs(-19.0f, 22.1f, 8.5f);
                    height += sebs(-14.3f, 10.5f, -19.1f);
                    height += sebs(-1.0f, -22.6f, 15.9f);
                    height += sebs(-19.9f, 4.9f, -3.1f);
                    height += sebs(13.9f, -15.3f, 5.6f);
                    height += sebs(13.3f, -22.6f, -18.0f);
                    height += sebs(23.9f, -20.3f, -20.5f);
                    height += sebs(1.5f, -0.6f, 7.3f);
                    height += sebs(11.7f, -16.1f, 22.0f);
                    height += sebs(-0.9f, 12.9f, -1.8f);
                    height += sebs(7.2f, 18.9f, -4.6f);
                    height += sebs(-9.3f, -20.4f, -19.1f);
                    height += sebs(-3.4f, 3.6f, -16.7f);
                    height += sebs(-12.5f, 8.7f, 13.2f);
                    height += sebs(9.4f, -2.4f, 22.9f);
                    height += sebs(-10.0f, -14.1f, -24.7f);
                    height += sebs(-2.9f, -15.1f, -20.0f);
                    height += sebs(1.6f, 11.3f, 4.4f);
                    height += sebs(-16.1f, -2.0f, 13.4f);
                    height += sebs(-19.1f, 22.1f, 1.3f);
                    height += sebs(4.2f, 4.3f, -4.8f);
                    height += sebs(24.6f, 20.0f, -0.2f);
                    height += sebs(-19.6f, -8.4f, -21.7f);
                    height += sebs(-11.3f, 4.1f, 12.0f);
                    height += sebs(-23.1f, -11.5f, -15.4f);
                    height += sebs(24.8f, 3.6f, -4.5f);
                    height += sebs(-24.9f, -24.4f, 5.4f);
                    height += sebs(-19.9f, 2.2f, -8.3f);
                    height += sebs(9.6f, 11.1f, 14.7f);
                    height += sebs(-2.1f, 17.0f, 11.8f);
                    height += sebs(24.2f, -3.9f, -8.9f);
                    height += sebs(-5.6f, -4.3f, -13.9f);
                    height += sebs(19.2f, 1.1f, 2.7f);
                    height += sebs(22.5f, 14.8f, -18.2f);
                    height += sebs(9.5f, 16.7f, -4.7f);
                    height += sebs(19.2f, 16.6f, 23.8f);
                    height += sebs(-10.3f, 16.7f, 24.5f);
                    height += sebs(20.1f, 21.8f, 1.6f);
                    height += sebs(-13.1f, 6.4f, -12.3f);
                    height += sebs(-23.4f, -20.7f, -20.3f);
                    height += sebs(13.4f, -21.5f, 0.8f);
                    height += sebs(-20.5f, -2.1f, 21.5f);
                    height += sebs(-9.4f, -8.0f, -2.4f);
                    height += sebs(18.3f, -10.5f, -12.6f);
                    height += sebs(-25.0f, 24.1f, -20.9f);
                    height += sebs(-4.7f, 18.2f, 20.7f);

                    height /= 25.0f;

                    return height;
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