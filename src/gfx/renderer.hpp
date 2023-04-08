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

    private:
        void initializeRenderer();

        Window window;

        // Vulkan Boilerplate
        std::shared_ptr<vulkan::Instance>     instance;
        std::shared_ptr<vk::UniqueSurfaceKHR> draw_surface;
        std::shared_ptr<vulkan::Device>       device;
        std::shared_ptr<vulkan::Allocator>    allocator;

        // Rendering Boilerplate
        std::unique_ptr<vulkan::Swapchain> swapchain;
        std::unique_ptr<vulkan::Image2D>   depth_buffer;

        // Pipelines!

        // vector of Polymorphic pipeline class
        // do something to auto matically dont do as much binding

        // Make the restriction that each pipeline can only be rendered from one
        // thread at at time.

        // array of frames to render.

        //

        // TODO: make a vulkan renderer class that owns references to these
        // make a polymorphic pipeline class that has a constructor (initalize
        // textures and stuff) a bind call/ and a draw thing class that

    }; // class Renderer
} // namespace gfx

#endif // SRC_GFX_RENDERER_HPP
