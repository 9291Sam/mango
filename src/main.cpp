#include "gfx/renderer.hpp"
#include "util/log.hpp"
#include "util/threads.hpp"
#include <exception>
#include <thread>

int main()
{
    try
    {
        auto [s, r] = util::mpmc::create<int>();

        s.send(3);

        util::logLog("{}", r.receive().value());

        gfx::Renderer renderer {};
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }

    util::logTrace("mango exited successfully");
}
