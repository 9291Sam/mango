#include "renderer.hpp"
#include "util/log.hpp"

namespace gfx
{
    Renderer::Renderer()
        : window   {{1'200, 1'200}, "Mango"}
        , instance {nullptr}
        , device   {nullptr}
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

        this->draw_surface = this->window.createSurface(**this->instance);

        this->device = std::make_shared<vulkan::Device>(
            this->instance, *this->draw_surface
        );

        util::logLog("Renderer initialization complete");
    }

    Renderer::~Renderer() {}
} // namespace gfx
