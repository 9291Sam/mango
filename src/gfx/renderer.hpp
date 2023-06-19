#ifndef SRC_GFX_RENDERER_HPP
#define SRC_GFX_RENDERER_HPP

#include "object.hpp" // TODO: remove?
#include "vulkan/pipelines.hpp"
#include "window.hpp"
#include <map>
#include <memory>
#include <span>

namespace gfx
{
    class Object;
    class VertexObject;
    class Camera;

    namespace vulkan
    {
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
        Renderer();
        ~Renderer();

        Renderer(const Renderer&)             = delete;
        Renderer(Renderer&&)                  = delete;
        Renderer& operator= (const Renderer&) = delete;
        Renderer& operator= (Renderer&&)      = delete;

        // TODO: make all of these methods thread safe

        bool shouldClose() const;

        float         getDeltaTimeSeconds() const;
        float         getActionAmount(Window::Action) const;
        Window::Delta getMouseDelta() const;
        void          drawObjects(const Camera&, std::span<const Object*>);

        vk::Extent2D getExtent() const;

    private:
        void resize();
        void initializeRenderer();

        friend gfx::Object;

        Window window;

        // Vulkan Boilerplate
        std::shared_ptr<vulkan::Instance>     instance;
        std::shared_ptr<vk::UniqueSurfaceKHR> draw_surface;
        std::shared_ptr<vulkan::Device>       device;
        std::shared_ptr<vulkan::Allocator>    allocator;

        // Rendering Boilerplate
        std::shared_ptr<vulkan::Swapchain>  swapchain;
        std::shared_ptr<vulkan::Image2D>    depth_buffer;
        std::shared_ptr<vulkan::RenderPass> render_pass;

        // Pipelines
        std::map<vulkan::PipelineType, vulkan::Pipeline> pipeline_map;

        // Flying frames
        static constexpr std::size_t MaxFramesInFlight = 2;
        std::size_t                  render_index;
        std::array<std::unique_ptr<Frame>, MaxFramesInFlight> frames;
    }; // class Renderer
} // namespace gfx

#endif // SRC_GFX_RENDERER_HPP
