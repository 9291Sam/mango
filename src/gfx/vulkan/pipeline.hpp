#ifndef SRC_GFX_VULKAN_PIPELINE_HPP
#define SRC_GFX_VULKAN_PIPELINE_HPP

#include "includes.hpp"
#include "util/log.hpp"
#include <memory>
#include <span>

namespace gfx::vulkan
{
    // TODO: replace with a builder!
    class Device;
    class RenderPass;
    class Swapchain;
    class DescriptorPool;

    // TODO: expand to be not shit
    // TODO: this may be a good PipelineBase class that just needs to be
    // specialized
    class Pipeline
    {
    public:
        static vk::UniqueShaderModule
        createShaderFromFile(vk::Device, const char* filePath);

    public:
        Pipeline(
            std::shared_ptr<Device>,
            std::shared_ptr<Swapchain>,
            std::shared_ptr<RenderPass>,
            std::shared_ptr<DescriptorPool>,
            std::span<const vk::PipelineShaderStageCreateInfo>,
            vk::UniquePipelineLayout);
        ~Pipeline() = default;

        Pipeline(const Pipeline&)             = delete;
        Pipeline(Pipeline&&)                  = delete;
        Pipeline& operator= (const Pipeline&) = delete;
        Pipeline& operator= (Pipeline&&)      = delete;

        [[nodiscard]] vk::Pipeline       operator* () const;
        [[nodiscard]] vk::PipelineLayout getLayout() const;

    private:
        std::shared_ptr<Device>         device;
        std::shared_ptr<Swapchain>      swapchain;
        std::shared_ptr<RenderPass>     render_pass;
        std::shared_ptr<DescriptorPool> descriptor_pool;
        vk::UniquePipelineLayout        layout;
        vk::UniquePipeline              pipeline;
    };

} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_PIPELINE_HPP
