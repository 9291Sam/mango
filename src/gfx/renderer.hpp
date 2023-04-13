#ifndef SRC_GFX_RENDERER_HPP
#define SRC_GFX_RENDERER_HPP

#include "vulkan/instance.hpp"
#include "window.hpp"
#include <memory>

namespace gfx
{
    namespace vulkan
    {
        class Instance;
        class Device;
        class Allocator;
        class Swapchain;
        class Image2D;
        class RenderPass;
        class Pipeline;
        class DescriptorPool;
    } // namespace vulkan

    class Renderer
    {
    public:
        Renderer();
        ~Renderer() = default;

        Renderer(const Renderer&)             = delete;
        Renderer(Renderer&&)                  = delete;
        Renderer& operator= (const Renderer&) = delete;
        Renderer& operator= (Renderer&&)      = delete;

    private:
        void initializeRenderer();

        Window window;

        // Vulkan Boilerplate
        std::shared_ptr<vulkan::Instance>     instance;
        std::shared_ptr<vk::UniqueSurfaceKHR> draw_surface;
        std::shared_ptr<vulkan::Device>       device;
        std::shared_ptr<vulkan::Allocator>    allocator;

        // Rendering Boilerplate
        std::shared_ptr<vulkan::Swapchain>  swapchain;
        std::shared_ptr<vulkan::Image2D>    depth_buffer;
        std::unique_ptr<vulkan::RenderPass> render_pass;

        // Pipelines
        std::shared_ptr<vulkan::DescriptorPool> descriptor_pool;

        // Rendering frames
    }; // class Renderer
} // namespace gfx

#endif // SRC_GFX_RENDERER_HPP
