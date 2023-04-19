#include "gfx/renderer.hpp"
#include "util/log.hpp"

int main()
{
    util::logLog("mango started");

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

    util::logLog("mango exited successfully");
}
