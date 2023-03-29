#include "renderer.hpp"
#include "util/log.hpp"

namespace gfx
{
    Renderer::Renderer()
        : window {
            {1'200, 1'200},
            "Mango"}, 
         instance {nullptr}
    {
        const vk::DynamicLoader         dl;
        const PFN_vkGetInstanceProcAddr dynVkGetInstanceProcAddr =
            dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"
            );
        VULKAN_HPP_DEFAULT_DISPATCHER.init(dynVkGetInstanceProcAddr);

        this->instance = std::make_unique<vulkan::Instance>(
            dynVkGetInstanceProcAddr,
            [&](vk::Instance instance_)
            {
                VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_);
            }
        );

        util::logLog("Renderer initialization complete");
    }

    Renderer::~Renderer() {}
} // namespace gfx
