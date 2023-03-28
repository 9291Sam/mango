#include "gfx/renderer.hpp"
#include "util/log.hpp"
#include <exception>

int main()
{
    try
    {
        gfx::Renderer renderer {};
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }
}
