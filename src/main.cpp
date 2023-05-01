#include "game/game.hpp"
#include "gfx/renderer.hpp"
#include "util/log.hpp"

int main()
{
    util::logLog("mango started");

    try
    {
        std::shared_ptr<gfx::Renderer> renderer =
            std::make_shared<gfx::Renderer>();

        game::Game game {renderer};

        while (!renderer->shouldClose())
        {
            game.tick();
        }
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }

    util::logLog("mango exited successfully");
}
