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
        , world {this->renderer}
    {
        this->entities.push_back(std::make_unique<entity::Cube>(
            this->renderer, glm::vec3 {0.0f, 12.5f, 0.0f}));

        this->entities.push_back(std::make_unique<entity::DiskEntity>(
            this->renderer, "../models/gizmo.obj"));

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
