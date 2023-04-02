#include "allocator.hpp"
#include "device.hpp"
#include "instance.hpp"

namespace gfx::vulkan
{
    Allocator::Allocator(
        std::shared_ptr<Instance> instance_,
        std::shared_ptr<Device>   device_,
        PFN_vkGetInstanceProcAddr dynVkGetInstanceProcAddr,
        PFN_vkGetDeviceProcAddr   dynVkGetDeviceProcAddr
    )
        : instance {std::move(instance_)}
        , device {std::move(device_)}
    {
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
            vk::to_string(vk::Result {result})
        );
    }

    Allocator::~Allocator()
    {
        vmaDestroyAllocator(this->allocator);
    }

    VmaAllocator Allocator::operator* () const
    {
        return this->allocator;
    }
} // namespace gfx::vulkan
