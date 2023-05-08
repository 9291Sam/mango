#include "game.hpp"
#include "entity/cube.hpp"
#include "entity/disk_object.hpp"
#include "world/voxel.hpp"
#include <gfx/renderer.hpp>
#include <util/log.hpp>

namespace game
{
    Game::Game(std::shared_ptr<gfx::Renderer> renderer_)
        : renderer {std::move(renderer_)}
        , entities {}
        , camera {{-30.0f, 20.0f, -20.0f}}
        , world {this->renderer, 78234789234}
    {
        this->entities.push_back(std::make_unique<entity::Cube>(
            this->renderer,
            gfx::Transform {
                .translation {0.0f, 12.5f, 0.0f},
                .rotation {1.0f, 0.0f, 0.0f, 0.0f},
                .scale {1.0f, 1.0f, 1.0f}}));

        this->entities.push_back(std::make_unique<entity::DiskObject>(
            this->renderer, "../models/gizmo.obj"));

        this->camera.addPitch(0.192699082f);
        this->camera.addYaw(2.04719755f);
    }

    Game::~Game() {}

    void Game::tick()
    {
        const float deltaTime = this->renderer->getDeltaTimeSeconds();
        std::vector<const gfx::Object*> drawObjects {};

        for (const std::unique_ptr<entity::Entity>& s : this->entities)
        {
            s->tick(deltaTime);

            for (const gfx::Object* obj : s->lend())
            {
                drawObjects.push_back(obj);
            }
        }

        for (const gfx::Object* o : this->world.lend())
        {
            drawObjects.push_back(o);
        }

        this->renderer->updateCamera(this->camera);

        this->renderer->drawObjects(camera, drawObjects);
    }

} // namespace game
