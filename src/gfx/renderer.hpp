#ifndef SRC_GFX_RENDERER_HPP
#define SRC_GFX_RENDERER_HPP

#include "vulkan/allocator.hpp"
#include "vulkan/device.hpp"
#include "vulkan/instance.hpp"
#include "window.hpp"
#include <memory>

namespace gfx
{
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
        Window window;

        // Vulkan boilerplate objects ?? maybe combine into0 one bigger class?
        std::shared_ptr<vulkan::Instance>  instance;
        vk::UniqueSurfaceKHR               draw_surface;
        std::shared_ptr<vulkan::Device>    device;
        std::shared_ptr<vulkan::Allocator> allocator;

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
