#ifndef SRC_GFX_VULKAN_SWAPCHAIN_HPP
#define SRC_GFX_VULKAN_SWAPCHAIN_HPP

#include "includes.hpp"
#include <memory>

namespace gfx::vulkan
{
    class Device;

    class Swapchain
    {
    public:
        Swapchain(
            std::shared_ptr<Device>,
            std::shared_ptr<vk::UniqueSurfaceKHR>,
            vk::Extent2D
        );
        ~Swapchain() = default;

        Swapchain()                             = delete;
        Swapchain(const Swapchain&)             = delete;
        Swapchain(Swapchain&&)                  = delete;
        Swapchain& operator= (const Swapchain&) = delete;
        Swapchain& operator= (Swapchain&&)      = delete;

        [[nodiscard]] vk::SwapchainKHR operator* () const;

        [[nodiscard]] vk::Extent2D         getExtent() const;
        [[nodiscard]] vk::SurfaceFormatKHR getSurfaceFormat() const;

        [[nodiscard]] auto getRenderTargets() const
            -> const std::vector<vk::UniqueImageView>&;

    private:
        std::shared_ptr<Device>               device;
        std::shared_ptr<vk::UniqueSurfaceKHR> window_surface;

        vk::Extent2D         extent;
        vk::SurfaceFormatKHR format;

        vk::UniqueSwapchainKHR           swapchain;
        std::vector<vk::Image>           images;
        std::vector<vk::UniqueImageView> image_views;

    }; // class Swapchain

} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_SWAPCHAIN_HPP
