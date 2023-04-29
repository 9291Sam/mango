#ifndef SRC_GFX_RENDERER_HPP
#define SRC_GFX_RENDERER_HPP

#include "window.hpp"
#include <memory>
#include <span>

namespace gfx
{
    class Object;
    class VertexObject;
    class Camera;

    namespace vulkan
    {
        // TODO: organize
        class Instance;
        class Buffer;
        class Device;
        class Allocator;
        class Swapchain;
        class Image2D;
        class RenderPass;
        class FlatPipeline;
        class Pipeline;
        class DescriptorPool;
        struct Vertex;
        using Index = std::uint32_t;

    } // namespace vulkan

    class Frame;

    class Renderer
    {
    public:
        // TODO: dont forget to actually create the pipeline!
        enum class PipelineType : std::size_t
        {
            FlatPipeline     = 0,
            MaxPipelineValue = 1,
        };

        static constexpr std::size_t PipelineNumber =
            static_cast<std::size_t>(PipelineType::MaxPipelineValue);
    public:
        Renderer();
        ~Renderer();

        Renderer(const Renderer&)             = delete;
        Renderer(Renderer&&)                  = delete;
        Renderer& operator= (const Renderer&) = delete;
        Renderer& operator= (Renderer&&)      = delete;

        [[nodiscard]] Object createObject(
            PipelineType,
            std::span<const vulkan::Vertex>,
            std::span<const vulkan::Index>) const;

        [[nodiscard]] bool  shouldClose() const;
        [[nodiscard]] float getDeltaTimeSeconds() const;

        void updateCamera(Camera&);
        void drawObjects(const Camera&, std::span<const Object*>);

    private:
        void resize();
        void initializeRenderer();

        Window window;

        // Vulkan Boilerplate
        std::shared_ptr<vulkan::Instance>       instance;
        std::shared_ptr<vk::UniqueSurfaceKHR>   draw_surface;
        std::shared_ptr<vulkan::Device>         device;
        std::shared_ptr<vulkan::Allocator>      allocator;
        std::shared_ptr<vulkan::DescriptorPool> descriptor_pool;

        // Rendering Boilerplate
        std::shared_ptr<vulkan::Swapchain>  swapchain;
        std::shared_ptr<vulkan::Image2D>    depth_buffer;
        std::shared_ptr<vulkan::RenderPass> render_pass;

        // Pipelines
        std::array<std::unique_ptr<vulkan::Pipeline>, PipelineNumber> pipelines;

        // Flying frames
        static constexpr std::size_t MaxFramesInFlight = 2;
        std::size_t                  render_index;
        std::array<std::unique_ptr<Frame>, MaxFramesInFlight> frames;

        std::unique_ptr<vulkan::Buffer> vertex_buffer; // TODO: bad!
    };                                                 // class Renderer
} // namespace gfx

#endif // SRC_GFX_RENDERER_HPP
