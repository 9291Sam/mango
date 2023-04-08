#ifndef SRC_GFX_VULKAN_RENDER__PASS_HPP
#define SRC_GFX_VULKAN_RENDER__PASS_HPP

#include "includes.hpp"
#include <memory>

namespace gfx::vulkan
{
    class Device;
    class Image2D;
    class Swapchain;

    class RenderPass
    {
    public:

        RenderPass(
            std::shared_ptr<Device>,
            std::shared_ptr<Swapchain>,
            std::shared_ptr<Image2D> depthBuffer
        );
        ~RenderPass() = default;

        RenderPass()                              = delete;
        RenderPass(const RenderPass&)             = delete;
        RenderPass(RenderPass&&)                  = delete;
        RenderPass& operator= (const RenderPass&) = delete;
        RenderPass& operator= (RenderPass&&)      = delete;

        [[nodiscard]] vk::RenderPass operator* () const;

    private:
        std::shared_ptr<Device>    device;
        std::shared_ptr<Swapchain> swapchain;
        std::shared_ptr<Image2D>   depth_buffer;
        vk::UniqueRenderPass       render_pass;
    }; // class RenderPass

} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_RENDER__PASS_HPP
