#ifndef SRC_GFX_VULKAN_DEVICE_HPP
#define SRC_GFX_VULKAN_DEVICE_HPP

#include "includes.hpp"
#include "util/threads.hpp"
#include <compare>
#include <memory>

namespace gfx::vulkan
{
    class Instance;
    class Queue;

    // TODO: refactor to use thread ids to
    // make sure each queue is only accessed by one thread at a time
    // this alsp removes the requirement of mutexes (actually err no? idk (edge
    // case of the os scheduler))
    //
    // EVENTUAL SPLIT:
    // tmeporary of this in commit 4d996657b94869c1a54e056a97c24c8036818fc8
    // Best graphics queue family (swapchain submission)
    //
    // compute queues
    // transfer queues
    //
    class Device
    {
    public:

        Device(
            std::shared_ptr<Instance>,
            vk::SurfaceKHR,
            std::function<void(vk::Device)>
                dynamicLoaderInitializationCallback);
        ~Device() = default;

        Device(const Device&)             = delete;
        Device(Device&&)                  = delete;
        Device& operator= (const Device&) = delete;
        Device& operator= (Device&&)      = delete;

        [[nodiscard]] bool shouldBuffersStage() const;

        [[nodiscard]] vk::Device         asLogicalDevice() const;
        [[nodiscard]] vk::PhysicalDevice asPhysicalDevice() const;

        [[nodiscard]] std::uint32_t getQueueFamilyIndex() const;
        [[nodiscard]] vk::Queue     getQueue() const;

    private:
        std::shared_ptr<Instance> instance;
        vk::PhysicalDevice        physical_device;
        vk::UniqueDevice          logical_device;
        std::uint32_t             queue_family_index;
        vk::Queue                 queue;
        bool                      should_buffers_stage;
    }; // class Device

    /// A fully thread safe abstraction around a vk::Queue that allows for
    /// submission without
    class Queue
    {
    public:

        Queue(
            vk::Device,
            vk::Queue,
            vk::QueueFlags,
            bool          supportsSurface,
            std::uint32_t queueFamilyIndex);
        ~Queue() = default;

        Queue(const Queue&)             = delete;
        Queue(Queue&&)                  = delete;
        Queue& operator= (const Queue&) = delete;
        Queue& operator= (Queue&&)      = delete;

        /// This function is guaranteed to be finished calling by the time
        /// the access function returns.
        bool
            try_access(std::function<void(vk::Queue, vk::CommandBuffer)>) const;
        bool           isInUse() const;
        std::size_t    getNumberOfOperationsSupported() const;
        vk::QueueFlags getFlags() const;
        bool           getSurfaceSupport() const;

        std::strong_ordering operator<=> (const Queue& o) const;

    private:
        vk::QueueFlags        flags;
        bool                  supports_surface;
        vk::UniqueCommandPool command_pool;
        std::unique_ptr<util::Mutex<vk::Queue, vk::UniqueCommandBuffer>>
            queue_buffer_mutex;
    };
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_DEVICE_HPP
