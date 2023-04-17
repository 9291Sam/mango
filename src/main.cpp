#include "gfx/renderer.hpp"
#include "util/log.hpp"
#include <chrono>
#include <exception>
#include <thread>

int main()
{
    try
    {
        gfx::Renderer renderer {};

        using namespace std::chrono_literals;

        std::this_thread::sleep_for(1s);

        for (;;)
        {
            renderer.drawFrame();
            util::logTrace("Frame finished drawing");
            // util::panic("struck!");
        }
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }

    util::logTrace("mango exited successfully");
}
