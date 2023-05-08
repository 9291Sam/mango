#ifndef SRC_GFX_VULKAN_PIPELINE_HPP
#define SRC_GFX_VULKAN_PIPELINE_HPP

#include "includes.hpp"
#include "util/log.hpp"
#include <memory>
#include <optional>

namespace gfx::vulkan
{
    class Device;
    class RenderPass;
    class Swapchain;

    class Pipeline
    {
    public:
        static vk::UniqueShaderModule
        createShaderFromFile(vk::Device, const char* filePath);
    public:
        Pipeline(
            std::shared_ptr<Device>,
            std::shared_ptr<RenderPass>,
            std::shared_ptr<Swapchain>,
            std::optional<vk::UniqueShaderModule> fragmentShader,
            std::optional<vk::UniqueShaderModule> vertexShader);
        ~Pipeline() = default;

        Pipeline(const Pipeline&)             = delete;
        Pipeline(Pipeline&&)                  = delete;
        Pipeline& operator= (const Pipeline&) = delete;
        Pipeline& operator= (Pipeline&&)      = delete;

        [[nodiscard]] vk::Pipeline       operator* () const;
        [[nodiscard]] vk::PipelineLayout getLayout() const;

    private:
        std::shared_ptr<Device>     device;
        std::shared_ptr<RenderPass> render_pass;
        std::shared_ptr<Swapchain>  swapchain;
        vk::UniquePipelineLayout    layout;
        vk::UniquePipeline          pipeline;
    };

} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_PIPELINE_HPP
