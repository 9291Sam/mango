#include "game.hpp"
#include "system/cube.hpp"
#include <gfx/renderer.hpp>

namespace game
{
    Game::Game(std::shared_ptr<gfx::Renderer> renderer_)
        : renderer {std::move(renderer_)}
        , systems {}
        , camera {{0.0f, 0.0f, 20.0f}}
    {
        this->systems.push_back(std::make_unique<system::Cube>(this->renderer));
    }

    Game::~Game() {}

    void Game::tick()
    {
        const float deltaTime = this->renderer->getDeltaTimeSeconds();
        std::vector<const gfx::Object*> drawObjects {};

        for (const std::unique_ptr<system::System>& s : this->systems)
        {
            s->tick(deltaTime);

            if (const system::ObjectSystem* o =
                    dynamic_cast<system::ObjectSystem*>(s.get()))
            {
                for (const gfx::Object* obj : o->lend())
                {
                    drawObjects.push_back(obj);
                }
            }
        }

        this->renderer->updateCamera(this->camera);

        this->renderer->drawObjects(camera, drawObjects);
    }

} // namespace game
