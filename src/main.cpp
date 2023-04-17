#include "gfx/renderer.hpp"
#include "util/log.hpp"
#include <chrono>
#include <exception>
#include <new>
#include <thread>

int main()
{
    try
    {
        gfx::Renderer renderer {};

        while (!renderer.shouldClose())
        {
            renderer.drawFrame();
        }
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }

    util::logTrace("mango exited successfully");
}
