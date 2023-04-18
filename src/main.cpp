#include "gfx/renderer.hpp"
#include "util/log.hpp"

int main()
{
    try
    {
        gfx::Renderer renderer {};

        for (;;)
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
