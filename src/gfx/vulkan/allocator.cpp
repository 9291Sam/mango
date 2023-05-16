#include "allocator.hpp"
#include "device.hpp"
#include "instance.hpp"

namespace gfx::vulkan
{
    Allocator::Allocator(
        std::shared_ptr<Instance> instance_,
        std::shared_ptr<Device>   device_,
        PFN_vkGetInstanceProcAddr dynVkGetInstanceProcAddr,
        PFN_vkGetDeviceProcAddr   dynVkGetDeviceProcAddr)
        : instance {std::move(instance_)}
        , device {std::move(device_)}
    {
        // TODO: make dynamic auto reallocing class
        std::unordered_map<vk::DescriptorType, std::uint32_t> descriptorMap {};
        descriptorMap[vk::DescriptorType::eStorageBuffer] = 3;

        this->descriptor_pool = vulkan::DescriptorPool::create(
            this->device, std::move(descriptorMap));

        VmaVulkanFunctions vulkanFunctions {};
        vulkanFunctions.vkGetInstanceProcAddr = dynVkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr   = dynVkGetDeviceProcAddr;

        const VmaAllocatorCreateInfo allocatorCreateInfo {
            .flags {},
            .physicalDevice {this->device->asPhysicalDevice()},
            .device {this->device->asLogicalDevice()},
            .preferredLargeHeapBlockSize {0}, // chosen by VMA
            .pAllocationCallbacks {nullptr},
            .pDeviceMemoryCallbacks {nullptr},
            .pHeapSizeLimit {nullptr},
            .pVulkanFunctions {&vulkanFunctions},
            .instance {**this->instance},
            .vulkanApiVersion {this->instance->getVulkanVersion()},
            .pTypeExternalMemoryHandleTypes {nullptr},
        };

        const VkResult result =
            vmaCreateAllocator(&allocatorCreateInfo, &this->allocator);

        util::assertFatal(
            result == VK_SUCCESS,
            "Failed to create allocator | Result: {}",
            vk::to_string(vk::Result {result}));
    }

    Allocator::~Allocator()
    {
        vmaDestroyAllocator(this->allocator);
    }

    bool Allocator::shouldBufferStage() const
    {
        return this->device->shouldBuffersStage();
    }

    VmaAllocator Allocator::operator* () const
    {
        return this->allocator;
    }

    DescriptorSet Allocator::allocateDescriptorSet(
        std::shared_ptr<DescriptorSetLayout> layout)
    {
        return this->descriptor_pool->allocate(std::move(layout));
    }
} // namespace gfx::vulkan
