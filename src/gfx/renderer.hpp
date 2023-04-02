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
        Window                            window;
        std::shared_ptr<vulkan::Instance> instance;
        vk::UniqueSurfaceKHR              draw_surface;
        std::shared_ptr<vulkan::Device>   device;
        std::shared_ptr<vulkan::Allocator> allocator;
    }; // class Renderer
} // namespace gfx

#endif // SRC_GFX_RENDERER_HPP
