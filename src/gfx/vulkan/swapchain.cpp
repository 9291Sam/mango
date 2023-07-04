#include "swapchain.hpp"
#include "device.hpp"
#include "util/log.hpp"
#include "vulkan/vulkan_enums.hpp"

namespace gfx::vulkan
{
    Swapchain::Swapchain(
        std::shared_ptr<Device>               device_,
        std::shared_ptr<vk::UniqueSurfaceKHR> windowSurface,
        vk::Extent2D                          extent_)
        : device {std::move(device_)}
        , window_surface {std::move(windowSurface)}
        , extent {extent_}
        , format {vk::SurfaceFormatKHR {
              .format {vk::Format::eB8G8R8A8Srgb},
              .colorSpace {vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear}}}
        , swapchain {nullptr}
        , images {}
        , image_views {}
    {
        // Ensure surface is compatible
        const auto availableSurfaces =
            this->device->asPhysicalDevice().getSurfaceFormatsKHR(
                **this->window_surface);

        if (std::ranges::find(availableSurfaces, this->format)
            == availableSurfaces.end())
        {
            util::panic("Failed to find ideal surface format!");
        }

        // Ensure image surface is able to be created
        // const vk::SurfaceCapabilitiesKHR surfaceCapabilities =
        //     device.asPhysicalDevice().getSurfaceCapabilitiesKHR(surface);
        // const vk::Extent2D minExtent = surfaceCapabilities.minImageExtent;
        // const vk::Extent2D maxExtent = surfaceCapabilities.maxImageExtent;
        // // seb::logTrace(
        // //     "minExtent {} {} | maxExtent {} {} | reqExtent {} {}",
        // //     minExtent.width, minExtent.height,
        // //     maxExtent.width, maxExtent.height,
        // //     this->extent.width, this->extent.height
        // // );
        // seb::assertFatal(
        //     minExtent.width <= this->extent.width,
        //     "Desired image extent width less than available)"
        // );
        // seb::assertFatal(
        //     minExtent.height <= this->extent.height,
        //     "Desired image extent height less than available)"
        // );
        // seb::assertFatal(
        //     maxExtent.width >= this->extent.width,
        //     "Desired image extent width greater than available"
        // );
        // seb::assertFatal(
        //     maxExtent.height >= this->extent.height,
        //     "Desired image extent height greater than available"
        // );

        // Select mailbox if available, else use fifo

        const vk::PresentModeKHR selectedPresentMode = [this]
        {
            const auto availablePresentModes =
                this->device->asPhysicalDevice().getSurfacePresentModesKHR(
                    **this->window_surface);

            return vk::PresentModeKHR::eFifo;

            if (std::ranges::find(
                    availablePresentModes, vk::PresentModeKHR::eMailbox)
                != availablePresentModes.cend())
            {
                return vk::PresentModeKHR::eMailbox;
            }
            else
            {
                return vk::PresentModeKHR::eFifo;
            }
        }();

        const vk::SurfaceCapabilitiesKHR surfaceCapabilities =
            this->device->asPhysicalDevice().getSurfaceCapabilitiesKHR(
                **this->window_surface);

        // clang-format off
        const vk::SwapchainCreateInfoKHR swapchainCreateInfoKHR
        {
            .sType         {vk::StructureType::eSwapchainCreateInfoKHR},
            .pNext         {nullptr},
            .flags         {},
            .surface       {**this->window_surface},
            .minImageCount {// :eyes:
                surfaceCapabilities.maxImageCount == 0
                ? surfaceCapabilities.minImageCount + 1
                : surfaceCapabilities.maxImageCount
            },
            .imageFormat           {this->format.format},
            .imageColorSpace       {this->format.colorSpace},
            .imageExtent           {this->extent},
            .imageArrayLayers      {1},
            .imageUsage            {vk::ImageUsageFlagBits::eColorAttachment},
            .imageSharingMode      {vk::SharingMode::eExclusive},
            .queueFamilyIndexCount {0},
            .pQueueFamilyIndices   {nullptr},
            .preTransform          {surfaceCapabilities.currentTransform},
            .compositeAlpha        {vk::CompositeAlphaFlagBitsKHR::eOpaque},
            .presentMode           {selectedPresentMode},
            .clipped               {true},
            .oldSwapchain          {nullptr}
        };
        // clang-format on

        this->swapchain =
            this->device->asLogicalDevice().createSwapchainKHRUnique(
                swapchainCreateInfoKHR);
        this->images = this->device->asLogicalDevice().getSwapchainImagesKHR(
            *this->swapchain);

        // image view initalization
        util::assertFatal(
            this->image_views.size() == 0, "Image views were not empty!");
        this->image_views.reserve(this->images.size());

        for (const vk::Image i : this->images)
        {
            const vk::ImageViewCreateInfo imageCreateInfo {
                .sType {vk::StructureType::eImageViewCreateInfo},
                .pNext {nullptr},
                .flags {},
                .image {i},
                .viewType {vk::ImageViewType::e2D},
                .format {this->format.format},
                .components {
                    .r {vk::ComponentSwizzle::eIdentity},
                    .g {vk::ComponentSwizzle::eIdentity},
                    .b {vk::ComponentSwizzle::eIdentity},
                    .a {vk::ComponentSwizzle::eIdentity},
                },
                .subresourceRange {
                    .aspectMask {vk::ImageAspectFlagBits::eColor},
                    .baseMipLevel {0},
                    .levelCount {1},
                    .baseArrayLayer {0},
                    .layerCount {1},
                },
            };

            this->image_views.push_back(
                this->device->asLogicalDevice().createImageViewUnique(
                    imageCreateInfo));
        }

        // util::logTrace("Successfully created swapchain!");
    }

    vk::SwapchainKHR Swapchain::operator* () const
    {
        return *this->swapchain;
    }

    vk::Extent2D Swapchain::getExtent() const
    {
        return this->extent;
    }

    vk::SurfaceFormatKHR Swapchain::getSurfaceFormat() const
    {
        return this->format;
    }

    const std::vector<vk::UniqueImageView>& Swapchain::getRenderTargets() const
    {
        return this->image_views;
    }
} // namespace gfx::vulkan
