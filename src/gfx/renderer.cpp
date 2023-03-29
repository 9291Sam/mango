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

        this->instance =
            std::make_unique<vulkan::Instance>(dynVkGetInstanceProcAddr);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(**this->instance);

        util::logLog("Renderer initialization complete");
    }

    Renderer::~Renderer() {}
} // namespace gfx
