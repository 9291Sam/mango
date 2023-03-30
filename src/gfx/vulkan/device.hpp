#ifndef SRC_GFX_VULKAN_DEVICE_HPP
#define SRC_GFX_VULKAN_DEVICE_HPP

#include "includes.hpp"

namespace gfx::vulkan
{
    class Queue;

    class Device
    {
    public:

        Device(vk::Instance, vk::SurfaceKHR);
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
        vk::PhysicalDevice physical_device;
        vk::UniqueDevice   logical_device;
        Queue              render_queue;
        Queue              compute_queue;
        Queue              transfer_queue;
        const bool         should_buffers_stage;
    }; // class Device

    /// A thread safe abstraction around a vk::Queue that allows
    class Queue
    {
    public:




    private:
        vk::Queue     queue;
        std::uint32_t index;
    }
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_DEVICE_HPP
