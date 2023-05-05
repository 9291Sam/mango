#include "game.hpp"
#include "entity/cube.hpp"
#include "entity/disk_object.hpp"
#include <gfx/renderer.hpp>

namespace game
{
    Game::Game(std::shared_ptr<gfx::Renderer> renderer_)
        : renderer {std::move(renderer_)}
        , entities {}
        , camera {{0.0f, 0.0f, 20.0f}}
    {
        this->entities.push_back(
            std::make_unique<entity::Cube>(this->renderer));
        this->entities.push_back(std::make_unique<entity::DiskObject>(
            this->renderer, "../models/gizmo.obj"));
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

        this->renderer->updateCamera(this->camera);

        this->renderer->drawObjects(camera, drawObjects);
    }

} // namespace game
