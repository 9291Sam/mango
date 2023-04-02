#include "device.hpp"
#include "instance.hpp"
#include "util/misc.hpp"
#include <ranges>
#include <span>

constexpr std::size_t                               MAX_QUEUE_COUNT {128};
static constexpr std::array<float, MAX_QUEUE_COUNT> QueuePriorities {1.0f};

std::vector<gfx::vulkan::QueueCreateInfo>
getDeviceQueueCreateInfos(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{
    std::vector<gfx::vulkan::QueueCreateInfo> tempCreateInfos;

    std::size_t idx = 0;
    for (vk::QueueFamilyProperties p : device.getQueueFamilyProperties())
    {
        if (p.queueCount > MAX_QUEUE_COUNT)
        {
            util::panic(
                "Found device with too many queues | Found: {} | Max: {}",
                p.queueCount,
                MAX_QUEUE_COUNT
            );
        }

        tempCreateInfos.push_back(gfx::vulkan::QueueCreateInfo {
            .queue_create_info {vk::DeviceQueueCreateInfo {
                .sType {vk::StructureType::eDeviceQueueCreateInfo},
                .pNext {nullptr},
                .flags {},
                .queueFamilyIndex {static_cast<std::uint32_t>(idx)},
                .queueCount {p.queueCount},
                .pQueuePriorities {QueuePriorities.data()},
            }},
            .family_index {static_cast<std::uint32_t>(idx)},
            .flags {p.queueFlags},
            .supports_surface {static_cast<bool>(device.getSurfaceSupportKHR(
                static_cast<std::uint32_t>(idx), surface
            ))}});

        util::logLog(
            "Idx: {} | Count: {} | Flags: {}",
            idx,
            p.queueCount,
            vk::to_string(p.queueFlags)
        );

        ++idx;
    }

    return tempCreateInfos;
}

std::vector<vk::DeviceQueueCreateInfo> // std::span be damned
convertToDeviceQueues(const std::vector<gfx::vulkan::QueueCreateInfo>& qS)
{
    std::vector<vk::DeviceQueueCreateInfo> tempCreateInfos {};
    tempCreateInfos.reserve(qS.size());

    for (const gfx::vulkan::QueueCreateInfo& q : qS)
    {
        tempCreateInfos.push_back(q.queue_create_info);
    }

    return tempCreateInfos;
}

std::size_t getDeviceRating(vk::PhysicalDevice device)
{
    std::size_t score = 0;

    const auto deviceLimits {device.getProperties().limits};

    score += deviceLimits.maxImageDimension2D;
    score += deviceLimits.maxImageDimension3D;

    return score;
}

namespace gfx::vulkan
{
    Device::Device(std::shared_ptr<Instance> instance_, vk::SurfaceKHR surface)
        : instance {std::move(instance_)}
        , physical_device {nullptr}
        , logical_device {nullptr}
        , graphics_surface_queue {}
        , compute_queue {}
        , transfer_queue {}
    {
        const auto physicalDevices =
            (**this->instance).enumeratePhysicalDevices();

        this->physical_device = *std::ranges::max_element(
            physicalDevices,
            [](vk::PhysicalDevice d1, vk::PhysicalDevice d2)
            {
                return getDeviceRating(d1) < getDeviceRating(d2);
            }
        );

        std::vector<QueueCreateInfo> queueCreateInfos =
            getDeviceQueueCreateInfos(this->physical_device, surface);

        std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos =
            convertToDeviceQueues(queueCreateInfos);

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
            .queueCreateInfoCount {
                static_cast<std::uint32_t>(deviceQueueCreateInfos.size())},
            .pQueueCreateInfos {deviceQueueCreateInfos.data()},
            .enabledLayerCount {0}, // deprecated
            .ppEnabledLayerNames {nullptr},
            .enabledExtensionCount {
                static_cast<std::uint32_t>(deviceExtensions.size())},
            .ppEnabledExtensionNames {deviceExtensions.data()},
            .pEnabledFeatures {&deviceFeatures},
        };

        this->logical_device =
            this->physical_device.createDeviceUnique(deviceCreateInfo);

        for (const QueueCreateInfo& q : queueCreateInfos)
        {
            for (std::size_t i = 0; i < q.queue_create_info.queueCount; ++i)
            {
                std::shared_ptr<Queue> queuePtr = std::make_shared<Queue>(
                    this->logical_device->getQueue(
                        q.family_index, static_cast<std::uint32_t>(i)
                    ),
                    q.flags,
                    q.supports_surface
                );

                if (queuePtr->getFlags() & vk::QueueFlagBits::eGraphics)
                {
                    if (!queuePtr->getSurfaceSupport())
                    {
                        util::panic(
                            "Found graphics queue without surface support!"
                        );
                    }

                    this->graphics_surface_queue.push_back(queuePtr);
                }

                if (queuePtr->getFlags() & vk::QueueFlagBits::eCompute)
                {
                    this->compute_queue.push_back(queuePtr);
                }

                if (queuePtr->getFlags() & vk::QueueFlagBits::eTransfer)
                {
                    this->transfer_queue.push_back(queuePtr);
                }

                util::assertFatal(
                    queuePtr.use_count() > 1,
                    "Queue was not allocated | Flags: {}",
                    vk::to_string(queuePtr->getFlags())
                );
            }
        }

        util::logLog(
            "Size of device queues Graphics & Surface: {} | Compute: {} | "
            "Transfer: {}",
            this->graphics_surface_queue.size(),
            this->compute_queue.size(),
            this->transfer_queue.size()
        );

        std::ranges::for_each(
            this->graphics_surface_queue,
            [](const std::shared_ptr<Queue>& q)
            {
                fmt::println("Queue: {}", static_cast<std::string>(*q));
            }
        );
        fmt::println("|||");
        std::ranges::for_each(
            this->compute_queue,
            [](const std::shared_ptr<Queue>& q)
            {
                fmt::println("Queue: {}", static_cast<std::string>(*q));
            }
        );
        fmt::println("|||");
        std::ranges::for_each(
            this->transfer_queue,
            [](const std::shared_ptr<Queue>& q)
            {
                fmt::println("Queue: {}", static_cast<std::string>(*q));
            }
        );

        std::sort(
            this->graphics_surface_queue.begin(),
            this->graphics_surface_queue.end()
        );
        std::sort(this->compute_queue.begin(), this->compute_queue.end());
        std::sort(this->transfer_queue.begin(), this->transfer_queue.end());

        std::ranges::for_each(
            this->graphics_surface_queue,
            [](const std::shared_ptr<Queue>& q)
            {
                fmt::println("Queue: {}", static_cast<std::string>(*q));
            }
        );
        fmt::println("|||");
        std::ranges::for_each(
            this->compute_queue,
            [](const std::shared_ptr<Queue>& q)
            {
                fmt::println("Queue: {}", static_cast<std::string>(*q));
            }
        );
        fmt::println("|||");
        std::ranges::for_each(
            this->transfer_queue,
            [](const std::shared_ptr<Queue>& q)
            {
                fmt::println("Queue: {}", static_cast<std::string>(*q));
            }
        );
    }

    Queue::Queue(vk::Queue queue, vk::QueueFlags flags_, bool supportsSurface_)
        : flags {flags_}
        , supports_surface {supportsSurface_}
        , queue_mutex {queue}
    {}

    void Queue::access(std::function<void(vk::Queue)> func) const
    {
        this->queue_mutex.lock(func);
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
