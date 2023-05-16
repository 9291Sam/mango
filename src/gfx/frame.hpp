#ifndef SRC_GFX_FRAME_HPP
#define SRC_GFX_FRAME_HPP

#include "vulkan/includes.hpp"
#include "vulkan/pipelines.hpp"
#include <map>
#include <memory>
#include <span>

namespace gfx
{
    class Camera;
    class Object;

    namespace vulkan
    {
        class Buffer;
        class Device;
        class Swapchain;
        class RenderPass;
        class FlatPipeline;
        class Pipeline;
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
        [[nodiscard]] bool render(
            const Camera&,
            vk::Extent2D windowSize,
            const std::map<vulkan::PipelineType, vulkan::Pipeline>&,
            std::span<const Object*>);

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
