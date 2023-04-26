#ifndef SRC_GFX_VULKAN_PIPELINE_HPP
#define SRC_GFX_VULKAN_PIPELINE_HPP

#include "includes.hpp"
#include "util/log.hpp"
#include <concepts>
#include <memory>
#include <span>

namespace gfx::vulkan
{
    // TODO: replace with a builder!
    class Device;
    class RenderPass;
    class Swapchain;

    class PipelineBuilder
    {};

    class Pipeline
    {
    public:
        static vk::UniqueShaderModule
        createShaderFromFile(vk::Device, const char* filePath);
    public:
        virtual ~Pipeline();

        Pipeline(const Pipeline&)             = delete;
        Pipeline(Pipeline&&)                  = delete;
        Pipeline& operator= (const Pipeline&) = delete;
        Pipeline& operator= (Pipeline&&)      = delete;

        [[nodiscard]] virtual std::strong_ordering
                                         operator<=> (const Pipeline&) const;
        [[nodiscard]] vk::Pipeline       operator* () const;
        [[nodiscard]] vk::PipelineLayout getLayout() const;

    protected:
        Pipeline(
            std::shared_ptr<Device>,
            std::shared_ptr<Swapchain>,
            std::shared_ptr<RenderPass>,
            const PipelineBuilder&);

    private:
        std::shared_ptr<Device>     device;
        std::shared_ptr<Swapchain>  swapchain;
        std::shared_ptr<RenderPass> render_pass;
        vk::UniquePipelineLayout    layout;
        vk::UniquePipeline          pipeline;
    };

    class FlatPipeline final : public Pipeline
    {
    public:
        FlatPipeline(
            std::shared_ptr<Device>,
            std::shared_ptr<Swapchain>,
            std::shared_ptr<RenderPass>);
        ~FlatPipeline() override;
    };

} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_PIPELINE_HPP
