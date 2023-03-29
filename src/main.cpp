#include "gfx/renderer.hpp"
#include "util/log.hpp"
#include <exception>
#include <thread>

int main()
{
    try
    {
        // gfx::Renderer renderer {};
        const vk::DynamicLoader         dl;
        const PFN_vkGetInstanceProcAddr dynVkGetInstanceProcAddr =
            dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"
            );
        VULKAN_HPP_DEFAULT_DISPATCHER.init(dynVkGetInstanceProcAddr);

        auto instance =
            std::make_unique<gfx::vulkan::Instance>(dynVkGetInstanceProcAddr);

        util::logTrace("mango exited successfully");
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }
}
