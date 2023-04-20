#ifndef SRC_GFX_VULKAN_FRAME_HPP
#define SRC_GFX_VULKAN_FRAME_HPP

// TODO: move out into outer folder

#include "includes.hpp"
#include <memory>
#include <span>

namespace gfx::vulkan
{
    class Buffer;
    class Device;
    class Swapchain;
    class RenderPass;
    class FlatPipeline;

    // abstraction over all the random bits of code that are required to
    // actually draw frames to the screen,
    // its a mess, TODO: refactor
    class Frame
    {
    public:

        Frame(
            std::shared_ptr<Device>,
            std::shared_ptr<Swapchain>,
            std::shared_ptr<RenderPass>);
        ~Frame() = default;

        Frame(const Frame&)             = delete;
        Frame(Frame&&)                  = delete;
        Frame& operator= (const Frame&) = delete;
        Frame& operator= (Frame&&)      = delete;

        // @return {true}, is resize seeded
        [[nodiscard]] bool render(
            const std::vector<vk::UniqueFramebuffer>&,
            const FlatPipeline&,
            const Buffer& vertexBuffer);

    private:
        std::shared_ptr<Device>     device;
        std::shared_ptr<Swapchain>  swapchain;
        std::shared_ptr<RenderPass> render_pass;

        vk::UniqueSemaphore image_available;
        vk::UniqueSemaphore render_finished;
        vk::UniqueFence     frame_in_flight;
    }; // class Frame
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_FRAME_HPP
