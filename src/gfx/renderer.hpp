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
        class FlatPipeline;
        class DescriptorPool;
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

        [[noreturn]] void drawFrame();

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
        std::shared_ptr<vulkan::RenderPass> render_pass;

        // Pipelines
        std::shared_ptr<vulkan::DescriptorPool> descriptor_pool;
        std::unique_ptr<vulkan::FlatPipeline>   flat_pipeline; // TODO expand
        // TODO: reddo the pipeline to have a bunch of different bind points
        // i.e three bind functions if they need three descriptors and
        // also dont forget some runtime checking to ensure theyre not used
        // incorrectely also as a result use this to sort this stuff as a result
        // the pipelines are isolated from the pool, they dont care where the
        // descriptors come from

        // TODO: youre having trouble naming this, does this mean it needs a
        // restructure? Rendering frames
        static constexpr std::size_t MaxFramesInFlight = 2;
        std::size_t                  render_index;

        // each drawer will have a uniform buffer for each of the seperate
        // descriptors that need to be bound sinec they
    }; // class Renderer
} // namespace gfx

#endif // SRC_GFX_RENDERER_HPP
