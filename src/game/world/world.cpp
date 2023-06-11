#include "world.hpp"
#include "voxel_octree.hpp"
#include <chrono>
#include <functional>
#include <gfx/renderer.hpp>
#include <gfx/vulkan/pipelines.hpp>
#include <numbers>
#include <util/noise.hpp>

namespace game::world
{
    World::World(gfx::Renderer& renderer_)
        : renderer {renderer_}
    {
        auto begin = std::chrono::high_resolution_clock::now();

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

                std::function<std::int32_t(void)> height;

                height = [=]() -> std::int32_t
                {
                    std::int32_t workingHeight = 0;

                    workingHeight += util::perlin(util::Vector<float, 2> {
                                         normalizedX * 16, normalizedY * 16})
                                   * 64;

                    workingHeight += util::perlin(util::Vector<float, 2> {
                                         normalizedX * 8, normalizedY * 8})
                                   * 128;

                    workingHeight += util::perlin(util::Vector<float, 2> {
                                         normalizedX * 4, normalizedY * 4})
                                   * 256;

                    workingHeight += util::perlin(util::Vector<float, 2> {
                                         normalizedX * 2, normalizedY * 2})
                                   * 512;

                    // TODO: add seeds

                    return workingHeight / 4;
                };

                world::Position outputPosition {ox, height(), oy};

                // glm::vec4 color {
                //     std::abs(
                //         normalizedX * normalizedY * normalizedX *
                //         normalizedY),
                //     util::map(normalizedX, -1.0f, 1.0f, 0.3f, 0.7f),
                //     util::map(normalizedY, -1.0f, 1.0f, 0.3f, 0.7f),
                //     1.0f};

                glm::vec4 color {0.0f, 1.0f, 1.0f, 1.0f};

                color.g = std::fmod(
                    util::map(normalizedX, -1.0f, 1.0f, 0.0f, 0.5f), 1.0f);
                color.b = std::fmod(
                    util::map(normalizedY, -1.0f, 1.0f, 0.0f, 0.5f), 1.0f);

                if (outputPosition.y % 2 == 0)
                {
                    color.r = 0.125f;
                }

                this->octree.access(outputPosition) = world::Voxel {color};
            }
        }

        auto end = std::chrono::high_resolution_clock::now();

        util::logTrace(
            "World generation complete {}ms",
            std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                .count());

        auto [vertices, indices] = this->octree.draw();

        this->objects.push_back(this->renderer.createTriangulatedObject(
            gfx::vulkan::PipelineType::Flat,
            std::move(vertices),
            std::move(indices)));

        util::logTrace("World initialization complete");
    }

    std::vector<std::shared_ptr<gfx::Object>> World::draw() const
    {
        return this->objects;
    }
} // namespace game::world