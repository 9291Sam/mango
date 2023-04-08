#ifndef SRC_GFX_VULKAN_IMAGE_HPP
#define SRC_GFX_VULKAN_IMAGE_HPP

#include "includes.hpp"
#include <memory>

namespace gfx::vulkan
{
    class Allocator;
    class Device;

    class Image2D
    {
    public:

        Image2D(
            std::shared_ptr<Allocator>,
            std::shared_ptr<Device>,
            vk::Extent2D,
            vk::Format,
            vk::ImageUsageFlags,
            vk::ImageAspectFlags,
            vk::ImageTiling,
            vk::MemoryPropertyFlags
        );
        ~Image2D();

        Image2D(const Image2D&)             = delete;
        Image2D(Image2D&&)                  = delete;
        Image2D& operator= (const Image2D&) = delete;
        Image2D& operator= (Image2D&&)      = delete;

        [[nodiscard]] vk::ImageView   operator* () const;
        [[nodiscard]] vk::Format      getFormat() const;
        [[nodiscard]] vk::ImageLayout getLayout() const;

        void transitionLayout(
            vk::CommandBuffer,
            vk::ImageLayout        from,
            vk::ImageLayout        to,
            vk::PipelineStageFlags sourceStage,
            vk::PipelineStageFlags destinationStage,
            vk::AccessFlags        sourceAccess,
            vk::AccessFlags        destinationAccess
        );
        // void copyFromBuffer(vk::CommandBuffer, const Buffer&) const;

    private:
        std::shared_ptr<Allocator> allocator;
        std::shared_ptr<Device>    device;

        vk::Extent2D         extent;
        vk::Format           format;
        vk::ImageAspectFlags aspect;
        vk::ImageLayout      layout;

        vk::Image           image;
        VmaAllocation       memory;
        vk::UniqueImageView view;
    }; // class Image2D

} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_IMAGE_HPP
