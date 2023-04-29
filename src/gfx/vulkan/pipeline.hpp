#ifndef SRC_GFX_VULKAN_PIPELINE_HPP
#define SRC_GFX_VULKAN_PIPELINE_HPP

#include "includes.hpp"
#include "util/log.hpp"
#include <concepts>
#include <memory>
#include <optional>
#include <span>

namespace gfx::vulkan
{
    class Device;
    class RenderPass;
    class Swapchain;

    template<class T>
    concept PipelineVertex = requires {
        {
            T::getBindingDescription()
        } -> std::same_as<const vk::VertexInputBindingDescription*>;
        {
            T::getAttributeDescriptions()
        } -> std::same_as<
            const std::array<vk::VertexInputAttributeDescription, 4>*>;
    };

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
        ~Pipeline();

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
