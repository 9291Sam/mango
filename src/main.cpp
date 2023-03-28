#include "gfx/renderer.hpp"
#include "util/log.hpp"
#include <exception>
#include <thread>

int main()
{
    try
    {
        gfx::Renderer renderer {};
        std::this_thread::sleep_for(std::chrono::milliseconds {5'000});
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }
}
