#include "game/game.hpp"
#include "gfx/renderer.hpp"
#include "util/log.hpp"
#include "util/matrix.hpp"
#include "util/threads.hpp"
#include "util/uuid.hpp"
#include "util/vector.hpp"

int main()
{
    util::logLog("mango started");

    try
    {
        gfx::Renderer renderer {};

        game::Game game {renderer};

        while (!renderer.shouldClose())
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
