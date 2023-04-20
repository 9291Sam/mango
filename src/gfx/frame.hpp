#ifndef SRC_GFX_FRAME_HPP
#define SRC_GFX_FRAME_HPP

#include "vulkan/includes.hpp"
#include <memory>
#include <span>

namespace gfx
{
    namespace vulkan
    {
        class Buffer;
        class Device;
        class Swapchain;
        class RenderPass;
        class FlatPipeline;
    } // namespace vulkan

    // abstraction over all the random bits of code that are required to
    // actually draw frames to the screen,
    class Frame
    {
    public:

        Frame(
            std::shared_ptr<vulkan::Device>,
            std::shared_ptr<vulkan::Swapchain>,
            std::shared_ptr<vulkan::RenderPass>,
            std::shared_ptr<std::vector<vk::UniqueFramebuffer>>);
        ~Frame() = default;

        Frame(const Frame&)             = delete;
        Frame(Frame&&)                  = delete;
        Frame& operator= (const Frame&) = delete;
        Frame& operator= (Frame&&)      = delete;

        // @return {true}, is resize needed
        [[nodiscard]] bool
        render(const vulkan::FlatPipeline&, const vulkan::Buffer& vertexBuffer);

    private:
        std::shared_ptr<vulkan::Device>                     device;
        std::shared_ptr<vulkan::Swapchain>                  swapchain;
        std::shared_ptr<vulkan::RenderPass>                 render_pass;
        std::shared_ptr<std::vector<vk::UniqueFramebuffer>> framebuffers;

        vk::UniqueSemaphore image_available;
        vk::UniqueSemaphore render_finished;
        vk::UniqueFence     frame_in_flight;
    }; // class Frame
} // namespace gfx

#endif // SRC_GFX_FRAME_HPP
