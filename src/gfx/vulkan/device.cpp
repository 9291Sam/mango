#include "device.hpp"
#include "instance.hpp"
#include "util/misc.hpp"
#include <ranges>
#include <span>

std::size_t getDeviceRating(vk::PhysicalDevice device)
{
    std::size_t score = 0;

    const auto deviceLimits {device.getProperties().limits};

    score += deviceLimits.maxImageDimension2D;
    score += deviceLimits.maxImageDimension3D;

    return score;
}

std::uint32_t findFamilyIndexOfGraphicsAndPresentQueue(
    vk::PhysicalDevice pD, vk::SurfaceKHR surface
)
{
    std::uint32_t idx = 0;

    for (auto q : pD.getQueueFamilyProperties())
    {
        if (!(q.queueFlags & vk::QueueFlagBits::eGraphics))
        {
            continue;
        }

        if (!(q.queueFlags & vk::QueueFlagBits::eTransfer))
        {
            continue;
        }

        if (!pD.getSurfaceSupportKHR(idx, surface))
        {
            continue;
        }

        return idx;
    }

    util::panic("Failed to find a suitable queue");
    std::unreachable();
}

namespace gfx::vulkan
{
    Device::Device(
        std::shared_ptr<Instance>       instance_,
        vk::SurfaceKHR                  surface,
        std::function<void(vk::Device)> dynamicLoaderInitializationCallback
    )
        : instance {std::move(instance_)}
        , physical_device {nullptr}
        , logical_device {nullptr}
        , queue_family_index {}
        , queue {}
    {
        const auto physicalDevices =
            (**this->instance).enumeratePhysicalDevices();

        this->physical_device = *std::max_element(
            physicalDevices.begin(),
            physicalDevices.end(),
            [](vk::PhysicalDevice d1, vk::PhysicalDevice d2)
            {
                return getDeviceRating(d1) < getDeviceRating(d2);
            }
        );

        this->queue_family_index = findFamilyIndexOfGraphicsAndPresentQueue(
            this->physical_device, surface
        );

        const float               One = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo {
            .sType {vk::StructureType::eDeviceQueueCreateInfo},
            .pNext {nullptr},
            .flags {},
            .queueFamilyIndex {this->queue_family_index},
            .queueCount {1},
            .pQueuePriorities {&One},
        };

        const std::vector<const char*> deviceExtensions {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
            VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif // __APPLE__
        };

        vk::PhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy          = true;
        deviceFeatures.fillModeNonSolid           = true;

        const vk::DeviceCreateInfo deviceCreateInfo {
            .sType {vk::StructureType::eDeviceCreateInfo},
            .pNext {nullptr},
            .flags {},
            .queueCreateInfoCount {1},
            .pQueueCreateInfos {&queueCreateInfo},
            .enabledLayerCount {0}, // deprecated
            .ppEnabledLayerNames {nullptr},
            .enabledExtensionCount {
                static_cast<std::uint32_t>(deviceExtensions.size())},
            .ppEnabledExtensionNames {deviceExtensions.data()},
            .pEnabledFeatures {&deviceFeatures},
        };

        this->logical_device =
            this->physical_device.createDeviceUnique(deviceCreateInfo);

        dynamicLoaderInitializationCallback(*this->logical_device);

        this->queue =
            this->logical_device->getQueue(this->queue_family_index, 0);

        this->should_buffers_stage = [this]
        {
            auto memoryProperties = this->physical_device.getMemoryProperties();

            std::vector<vk::MemoryType> memoryTypes;
            std::vector<vk::MemoryHeap> memoryHeaps;

            std::optional<std::size_t> idx_of_gpu_main_memory = std::nullopt;

            for (std::size_t i = 0; i < memoryProperties.memoryTypeCount; i++)
            {
                memoryTypes.push_back(memoryProperties.memoryTypes.at(i));
            }

            for (std::size_t i = 0; i < memoryProperties.memoryHeapCount; i++)
            {
                memoryHeaps.push_back(memoryProperties.memoryHeaps.at(i));
            }

            const vk::MemoryPropertyFlags desiredFlags =
                vk::MemoryPropertyFlagBits::eDeviceLocal
                | vk::MemoryPropertyFlagBits::eHostVisible
                | vk::MemoryPropertyFlagBits::eHostCoherent;

            for (auto t : memoryTypes)
            {
                if ((t.propertyFlags & desiredFlags) == desiredFlags)
                {
                    util::assertWarn(
                        !idx_of_gpu_main_memory.has_value(),
                        "There should only be one memory pool with "
                        "`desiredFlags`!"
                    );

                    idx_of_gpu_main_memory = t.heapIndex;
                }
            }

            if (idx_of_gpu_main_memory.has_value())
            {
                if (memoryProperties.memoryHeaps
                        .at(idx_of_gpu_main_memory.value())
                        .size
                    > 257 * 1024 * 1024)
                {
                    return false;
                }
            }
            return true;
        }();
    }

    vk::Device Device::asLogicalDevice() const
    {
        return *this->logical_device;
    }
    vk::PhysicalDevice Device::asPhysicalDevice() const
    {
        return this->physical_device;
    }

    Queue::Queue(
        vk::Device     device,
        vk::Queue      queue,
        vk::QueueFlags flags_,
        bool           supportsSurface,
        std::uint32_t  queueFamilyIndex
    )
        : flags {flags_}
        , supports_surface {supportsSurface}
        , command_pool {nullptr}
        , queue_buffer_mutex {nullptr}
    {
        const vk::CommandPoolCreateInfo commandPoolCreateInfo {
            .sType {vk::StructureType::eCommandPoolCreateInfo},
            .pNext {nullptr},
            .flags {vk::CommandPoolCreateFlagBits::eResetCommandBuffer},
            .queueFamilyIndex {queueFamilyIndex}};

        this->command_pool =
            device.createCommandPoolUnique(commandPoolCreateInfo);

        const vk::CommandBufferAllocateInfo commandBufferAllocateInfo {
            .sType {vk::StructureType::eCommandBufferAllocateInfo},
            .pNext {nullptr},
            .commandPool {*this->command_pool},
            .level {vk::CommandBufferLevel::ePrimary},
            .commandBufferCount {1}};

        this->queue_buffer_mutex =
            std::make_unique<util::Mutex<vk::Queue, vk::UniqueCommandBuffer>>(
                queue,
                std::move(
                    device
                        .allocateCommandBuffersUnique(commandBufferAllocateInfo)
                        .at(0)
                )
            );

        // initalize queue mutex
    }

    bool
    Queue::try_access(std::function<void(vk::Queue, vk::CommandBuffer)> func
    ) const
    {
        return this->queue_buffer_mutex->try_lock(
            [&](vk::Queue& queue, vk::UniqueCommandBuffer& commandBuffer)
            {
                commandBuffer->reset();

                func(queue, *commandBuffer);
            }
        );
    }

    std::size_t Queue::getNumberOfOperationsSupported() const
    {
        std::size_t number = 0;

        if (this->supports_surface)
        {
            ++number;
        }

        if (this->flags & vk::QueueFlagBits::eCompute)
        {
            ++number;
        }

        if (this->flags & vk::QueueFlagBits::eGraphics)
        {
            ++number;
        }

        if (this->flags & vk::QueueFlagBits::eTransfer)
        {
            ++number;
        }

        if (this->flags & vk::QueueFlagBits::eSparseBinding)
        {
            ++number;
        }

        return number;
    }

    vk::QueueFlags Queue::getFlags() const
    {
        return this->flags;
    }

    bool Queue::getSurfaceSupport() const
    {
        return this->supports_surface;
    }

    std::strong_ordering Queue::operator<=> (const Queue& o) const
    {
        return std::strong_order(
            this->getNumberOfOperationsSupported(),
            o.getNumberOfOperationsSupported()
        );
    }
} // namespace gfx::vulkan
