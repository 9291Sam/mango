#ifndef SRC_GFX_VULKAN_DEVICE_HPP
#define SRC_GFX_VULKAN_DEVICE_HPP

#include "includes.hpp"
#include "util/threads.hpp"
#include <compare>

namespace gfx::vulkan
{
    class Instance;
    class Queue;

    class Device
    {
    public:

        Device(std::shared_ptr<Instance>, vk::SurfaceKHR);
        ~Device() = default;

        Device(const Device&)             = delete;
        Device(Device&&)                  = delete;
        Device& operator= (const Device&) = delete;
        Device& operator= (Device&&)      = delete;

        [[nodiscard]] bool shouldBuffersStage() const;

        [[nodiscard]] vk::Device         asLogicalDevice() const;
        [[nodiscard]] vk::PhysicalDevice asPhysicalDevice() const;

        [[nodiscard]] const Queue& getRenderQueue() const;
        [[nodiscard]] const Queue& getComputeQueue() const;
        [[nodiscard]] const Queue& getTransferQueue() const;

    private:
        std::shared_ptr<Instance>           instance;
        vk::PhysicalDevice                  physical_device;
        vk::UniqueDevice                    logical_device;
        std::vector<std::shared_ptr<Queue>> graphics_surface_queue;
        std::vector<std::shared_ptr<Queue>> compute_queue;
        std::vector<std::shared_ptr<Queue>> transfer_queue;
        bool                                should_buffers_stage;
        [[maybe_unused]] std::byte          _padding[7];
    }; // class Device

    // TODO: does this need to be in the header
    struct QueueCreateInfo
    {
        vk::DeviceQueueCreateInfo  queue_create_info;
        std::uint32_t              family_index;
        vk::QueueFlags             flags;
        bool                       supports_surface;
        [[maybe_unused]] std::byte _padding[7];
    };

    /// A thread safe abstraction around a vk::Queue that allows for submission
    /// without
    class Queue
    {
    public:

        Queue(vk::Queue, vk::QueueFlags, bool supportsSurface);
        ~Queue() = default;

        Queue(const Queue&)             = delete;
        Queue(Queue&&)                  = delete;
        Queue& operator= (const Queue&) = delete;
        Queue& operator= (Queue&&)      = delete;

        // thread safe
        void           access(std::function<void(vk::Queue)>) const;
        bool           isInUse() const;
        std::size_t    getNumberOfOperationsSupported() const;
        vk::QueueFlags getFlags() const;
        bool           getSurfaceSupport() const;

        std::strong_ordering operator<=> (const Queue& o) const;

    private:
        vk::QueueFlags             flags;
        // std::uint32_t              index; // TODO: is this needed?
        bool                       supports_surface;
        [[maybe_unused]] std::byte _padding[3];
        util::Mutex<vk::Queue>     queue_mutex;
    };
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_DEVICE_HPP
