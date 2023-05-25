#include "game.hpp"
#include "entity/cube.hpp"
#include "entity/disk_entity.hpp"
#include <gfx/renderer.hpp>
#include <util/log.hpp>

namespace game
{
    Game::Game(gfx::Renderer& renderer_)
        : renderer {renderer_}
        , player {this->renderer, {-30.0f, 20.0f, -20.0f}}
        , entities {}
        , world {this->renderer, glm::vec3 {0.0f, 16.0f, 0.0f}, 10}
    {
        this->entities.push_back(std::make_unique<entity::Cube>(
            this->renderer, glm::vec3 {0.0f, 12.5f, 0.0f}));

        this->entities.push_back(std::make_unique<entity::DiskEntity>(
            this->renderer, "../models/gizmo.obj"));

        const std::size_t extent = (this->world.dimension / 2);

        for (std::size_t x : std::views::iota(-extent, extent))
        {
            for (std::size_t y : std::views::iota(-extent, extent))
            {
                const float normalizedX =
                    static_cast<float>(x) / this->world.dimension;
                const float normalizedY =
                    static_cast<float>(y) / this->world.dimension;

                const float pix = std::numbers::pi * 20;

                const std::size_t height = static_cast<std::size_t>(
                    (22 * std::sin(normalizedX * pix))
                    + (22 * std::cos(normalizedY * (pix))));

                this->world.insertVoxelAtPosition(
                    world::Voxel {
                        .linear_color {0.0f, normalizedX, normalizedY, 1.0f}},
                    world::LocalPosition {
                        .x {static_cast<std::int32_t>(x)},
                        .y {static_cast<std::int32_t>(height)},
                        .z {static_cast<std::int32_t>(y)}});
            }
        }

        this->player.getCamera().addPitch(0.418879019f);
        this->player.getCamera().addYaw(2.19911486f);
    }

    Game::~Game() {}

    void Game::tick()
    {
        this->player.tick();

        std::vector<const gfx::Object*> objects {};

        for (const std::unique_ptr<entity::Entity>& e : this->entities)
        {
            e->tick();

            for (const gfx::Object* obj : e->draw())
            {
                objects.push_back(obj);
            }
        }

        this->player.tick();

        auto worldObjects = this->world.draw(glm::vec3 {0.0f, 0.0f, 0.0f});

        for (std::shared_ptr<gfx::Object>& obj : worldObjects)
        {
            objects.push_back(obj.get());
        }

        this->renderer.drawObjects(this->player.getCamera(), objects);
    }

} // namespace game
