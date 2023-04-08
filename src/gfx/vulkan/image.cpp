#include "image.hpp"
#include "allocator.hpp"
#include "device.hpp"
#include <vk_mem_alloc.h>

namespace gfx::vulkan
{
    Image2D::Image2D(
        std::shared_ptr<Allocator> allocator_,
        std::shared_ptr<Device>    device_,
        vk::Extent2D               extent_,
        vk::Format                 format_,
        vk::ImageUsageFlags        usage,
        vk::ImageAspectFlags       aspect_,
        vk::ImageTiling            tiling,
        vk::MemoryPropertyFlags    memoryPropertyFlags
    )
        : allocator {std::move(allocator_)}
        , device {std::move(device_)}
        , extent {extent_}
        , format {format_}
        , aspect {aspect_}
    {
        const VkImageCreateInfo imageCreateInfo {
            .sType {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO},
            .pNext {nullptr},
            .flags {},
            .imageType {VK_IMAGE_TYPE_2D},
            .format {static_cast<VkFormat>(format)},
            .extent {VkExtent3D {
                .width {this->extent.width},
                .height {this->extent.height},
                .depth {1}}},
            .mipLevels {1},
            .arrayLayers {1},
            .samples {VK_SAMPLE_COUNT_1_BIT},
            .tiling {static_cast<VkImageTiling>(tiling)},
            .usage {static_cast<VkImageUsageFlags>(usage)},
            .sharingMode {VK_SHARING_MODE_EXCLUSIVE},
            .queueFamilyIndexCount {0},
            .pQueueFamilyIndices {nullptr},
            .initialLayout {VK_IMAGE_LAYOUT_UNDEFINED}};

        const VmaAllocationCreateInfo imageAllocationCreateInfo {
            .flags {},
            .usage {VMA_MEMORY_USAGE_AUTO},
            .requiredFlags {
                static_cast<VkMemoryPropertyFlags>(memoryPropertyFlags)},
            .preferredFlags {},
            .memoryTypeBits {},
            .pool {nullptr},
            .pUserData {nullptr},
            .priority {1.0f}};

        VkImage outputImage = nullptr;

        const VkResult result = vmaCreateImage(
            **this->allocator,
            &imageCreateInfo,
            &imageAllocationCreateInfo,
            &outputImage,
            &this->memory,
            nullptr
        );

        util::assertFatal(
            result == VK_SUCCESS,
            "Failed to allocate image memory {}",
            vk::to_string(vk::Result {result})
        );

        util::assertFatal(
            outputImage != nullptr, "Returned image was nullptr!"
        );

        this->image = vk::Image {outputImage};

        vk::ImageViewCreateInfo imageViewCreateInfo {
            .sType {vk::StructureType::eImageViewCreateInfo},
            .pNext {nullptr},
            .flags {},
            .image {this->image},
            .viewType {vk::ImageViewType::e2D},
            .format {this->format},
            .components {},
            .subresourceRange {vk::ImageSubresourceRange {
                .aspectMask {this->aspect},
                .baseMipLevel {0},
                .levelCount {1},
                .baseArrayLayer {0},
                .layerCount {1},
            }},
        };

        this->view = this->device->asLogicalDevice().createImageViewUnique(
            imageViewCreateInfo
        );
    }

    Image2D::~Image2D()
    {
        vmaDestroyImage(
            **this->allocator, static_cast<VkImage>(this->image), this->memory
        );
    }

    vk::ImageView Image2D::operator* () const
    {
        return *this->view;
    }

    vk::Format Image2D::getFormat() const
    {
        return this->format;
    }

    vk::ImageLayout Image2D::getLayout() const
    {
        return this->layout;
    }

    void Image2D::transitionLayout(
        vk::CommandBuffer      commandBuffer,
        vk::ImageLayout        from,
        vk::ImageLayout        to,
        vk::PipelineStageFlags sourceStage,
        vk::PipelineStageFlags destinationStage,
        vk::AccessFlags        sourceAccess,
        vk::AccessFlags        destinationAccess
    )
    {
        util::assertFatal(
            this->layout == from,
            "Incompatible layouts! {} | {}",
            vk::to_string(this->layout),
            vk::to_string(from)
        );

        const vk::ImageMemoryBarrier barrier {
            .sType {vk::StructureType::eImageMemoryBarrier},
            .pNext {nullptr},
            .srcAccessMask {sourceAccess},
            .dstAccessMask {destinationAccess},
            .oldLayout {from},
            .newLayout {to},
            .srcQueueFamilyIndex {VK_QUEUE_FAMILY_IGNORED},
            .dstQueueFamilyIndex {VK_QUEUE_FAMILY_IGNORED},
            .image {this->image},
            .subresourceRange {vk::ImageSubresourceRange {
                .aspectMask {this->aspect},
                .baseMipLevel {0},
                .levelCount {1},
                .baseArrayLayer {0},
                .layerCount {1},
            }},
        };

        commandBuffer.pipelineBarrier(
            sourceStage, destinationStage, {}, nullptr, nullptr, barrier
        );

        this->layout = to;
    }
} // namespace gfx::vulkan
