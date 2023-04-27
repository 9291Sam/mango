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
    class RenderPass;

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

    struct PipelineBuilder
    {
    public:
        using Self = std::remove_pointer_t<decltype(this)>;
    public:
        PipelineBuilder() {};

        // TODO: actually dont hack this together and do it right
        Self& withVertexShader(vk::UniqueShaderModule);
        Self& withFragmentShader(vk::UniqueShaderModule);
        Self& withLayout(vk::UniquePipelineLayout);

        vk::UniquePipeline build(const RenderPass&, const Swapchain&) const;

    private:
        std::optional<vk::UniqueShaderModule>   vertex_shader;
        std::optional<vk::UniqueShaderModule>   fragment_shader;
        std::optional<vk::UniquePipelineLayout> layout;
    };

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

        // TODO: add an id number
        [[nodiscard]] virtual std::strong_ordering
                                         operator<=> (const Pipeline&) const;
        [[nodiscard]] vk::Pipeline       operator* () const;
        [[nodiscard]] vk::PipelineLayout getLayout() const;

    protected:
        Pipeline(
            std::shared_ptr<Device>,
            std::shared_ptr<Swapchain>,
            std::shared_ptr<RenderPass>,
            PipelineBuilder);

    private:
        std::shared_ptr<Device> device;
        std::shared_ptr<Swapchain>
            swapchain; // TODO: make this a const reference
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
