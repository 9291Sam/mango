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
        using Self = PipelineBuilder;

        // TODO: actually dont hack this together and do it right
        auto withVertexShader(vk::UniqueShaderModule) -> Self&;
        auto withFragmentShader(vk::UniqueShaderModule) -> Self&;
        auto withLayout(vk::UniquePipelineLayout) -> Self&;

        Self transfer()
        {
            return PipelineBuilder {
                .vertex_shader {std::move(this->vertex_shader)},
                .fragment_shader {std::move(this->fragment_shader)},
                .layout {std::move(this->layout)},
            };
        }

        vk::UniquePipeline
        build(const Device&, const RenderPass&, const Swapchain&) const;

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

        [[nodiscard]] virtual std::strong_ordering
                                         operator<=> (const Pipeline&) const;
        [[nodiscard]] vk::Pipeline       operator* () const;
        [[nodiscard]] vk::PipelineLayout getLayout() const;

    protected:
        Pipeline(
            std::shared_ptr<Device>,
            std::shared_ptr<RenderPass>,
            std::shared_ptr<Swapchain>,
            PipelineBuilder,
            std::size_t id);

    private:
        std::shared_ptr<Device>     device;
        std::shared_ptr<RenderPass> render_pass;
        std::shared_ptr<Swapchain>
            swapchain; // TODO: make this a const reference + dont store for
                       // lifetime extensions
        vk::UniquePipelineLayout layout;
        vk::UniquePipeline       pipeline;
        std::size_t              id;
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
