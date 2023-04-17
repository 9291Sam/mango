#ifndef SRC_GFX_VULKAN_BUFFER_HPP
#define SRC_GFX_VULKAN_BUFFER_HPP

#include "includes.hpp"
#include <cstddef>
#include <memory>
#include <optional>
#include <span>

namespace gfx::vulkan
{
    class Allocator;
    class Device;

    class Buffer
    {
    public:

        Buffer(
            std::shared_ptr<Allocator>,
            std::size_t sizeBytes,
            vk::BufferUsageFlags,
            vk::MemoryPropertyFlags);
        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&);
        Buffer& operator= (const Buffer&) = delete;
        Buffer& operator= (Buffer&&);

        [[nodiscard]] vk::Buffer  operator* () const;
        [[nodiscard]] std::size_t sizeBytes() const;
        [[nodiscard]] void*       getMappedPtr() const;

        void write(std::span<const std::byte>) const;
        void copyFrom(const Buffer&, vk::CommandBuffer) const;

    private:
        void free();

        std::shared_ptr<Allocator> allocator;
        VkBuffer                   buffer;
        VmaAllocation              allocation;
        std::size_t                size;
        mutable void*              mapped_memory;
    };

    class StagedBuffer
    {
    public:

        StagedBuffer(
            const Device&,
            std::shared_ptr<Allocator>,
            std::size_t sizeBytes,
            vk::BufferUsageFlags);
        ~StagedBuffer() = default;

        StagedBuffer()                                = delete;
        StagedBuffer(const StagedBuffer&)             = delete;
        StagedBuffer(StagedBuffer&&)                  = default;
        StagedBuffer& operator= (const StagedBuffer&) = delete;
        StagedBuffer& operator= (StagedBuffer&&)      = default;

        void write(std::span<const std::byte>) const;
        void stage(vk::CommandBuffer) const;

    private:
        std::optional<Buffer> staging_buffer;
        Buffer                gpu_local_buffer;
    };
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_BUFFER_HPP
