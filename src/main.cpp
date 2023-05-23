#include "game/game.hpp"
#include "gfx/renderer.hpp"
#include "util/log.hpp"
#include "util/threads.hpp"
#include "util/uuid.hpp"

int main()
{
    util::logLog("mango started");

    const std::size_t FPS_LIMIT {240};
    const float       MIN_FRAME_TIME {1.0f / static_cast<float>(FPS_LIMIT)};
    const std::chrono::duration<float> MIN_DURATION {MIN_FRAME_TIME};

    try
    {
        gfx::Renderer renderer {};

        game::Game game {renderer};

        while (!renderer.shouldClose())
        {
            // std::chrono::duration<float> previous_render_time {
            // renderer.getDeltaTimeSeconds()};
            //
            // std::this_thread::sleep_for(MIN_DURATION - previous_render_time);
            game.tick();
        }
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }

    util::logLog("mango exited successfully");
}
