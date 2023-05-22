#include "game.hpp"
#include "cube.hpp"
#include "disk_entity.hpp"
#include <gfx/renderer.hpp>
#include <util/log.hpp>

namespace game
{
    Game::Game(gfx::Renderer& renderer_)
        : renderer {renderer_}
        , player {this->renderer, {-30.0f, 20.0f, -20.0f}}
        , entities {}
    {
        this->entities.push_back(std::make_unique<Cube>(
            this->renderer, glm::vec3 {0.0f, 12.5f, 0.0f}));

        this->entities.push_back(std::make_unique<DiskEntity>(
            this->renderer, "../models/gizmo.obj"));

        this->player.getCamera().addPitch(0.192699082f);
        this->player.getCamera().addYaw(2.04719755f);
    }

    Game::~Game() {}

    void Game::tick()
    {
        this->player.tick();

        std::vector<const gfx::Object*> objects {};

        for (const std::unique_ptr<Entity>& e : this->entities)
        {
            e->tick();

            for (const gfx::Object* obj : e->draw())
            {
                objects.push_back(obj);
            }
        }

        this->player.tick();

        // TODO: world!

        this->renderer.drawObjects(this->player.getCamera(), objects);
    }

} // namespace game
