#ifndef SRC_GFX_RENDERER_HPP
#define SRC_GFX_RENDERER_HPP

#include "vulkan/instance.hpp"
#include "window.hpp"
#include <memory>

namespace gfx
{
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
        class DescriptorPool;
        class Frame;
    } // namespace vulkan

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        Renderer(const Renderer&)             = delete;
        Renderer(Renderer&&)                  = delete;
        Renderer& operator= (const Renderer&) = delete;
        Renderer& operator= (Renderer&&)      = delete;

        [[nodiscard]] bool shouldClose() const;

        void drawFrame();

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

        // Pipelines TODO: abstract better
        std::unique_ptr<vulkan::FlatPipeline> flat_pipeline;

        // Flying frames
        static constexpr std::size_t MaxFramesInFlight = 2;
        std::size_t                  render_index;

        std::vector<vk::UniqueFramebuffer> framebuffers;
        std::array<std::unique_ptr<vulkan::Frame>, MaxFramesInFlight> frames;

        std::unique_ptr<vulkan::Buffer> vertex_buffer; // TODO: bad!
    };                                                 // class Renderer
} // namespace gfx

#endif // SRC_GFX_RENDERER_HPP
