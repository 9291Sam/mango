#ifndef SRC_GFX_VULKAN_PIPELINE_HPP
#define SRC_GFX_VULKAN_PIPELINE_HPP

#include "includes.hpp"
#include <memory>
#include <span>

namespace gfx::vulkan
{
    class Device;
    class RenderPass;
    class Swapchain;

    class Pipeline
    {
    public:
        static vk::UniqueShaderModule
            createShaderModule(vk::Device, std::span<const std::byte>);
    public:

        Pipeline(std::shared_ptr<Device>, std::shared_ptr<RenderPass>, std::shared_ptr<Swapchain>);
        ~Pipeline();

    private:
        // lots of private functions for code reuse

    }; // class Pipeline

    // concept PipelineClass;
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_PIPELINE_HPP
