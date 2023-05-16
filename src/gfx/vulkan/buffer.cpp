#include "buffer.hpp"
#include "allocator.hpp"
#include "device.hpp"

namespace gfx::vulkan
{
    Buffer::Buffer()
        : allocator {nullptr}
        , buffer {nullptr}
        , allocation {nullptr}
        , size {~0UZ}
        , mapped_memory {nullptr}
    {}

    Buffer::Buffer(
        std::shared_ptr<Allocator> allocator_,
        std::size_t                sizeBytes,
        vk::BufferUsageFlags       usage,
        vk::MemoryPropertyFlags    memoryPropertyFlags)
        : allocator {std::move(allocator_)}
        , buffer {nullptr}
        , allocation {nullptr}
        , size {sizeBytes}
        , mapped_memory {nullptr}
    {
        const VkBufferCreateInfo bufferCreateInfo {
            .sType {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO},
            .pNext {nullptr},
            .flags {},
            .size {this->size},
            .usage {static_cast<VkBufferUsageFlags>(usage)},
            .sharingMode {VK_SHARING_MODE_EXCLUSIVE},
            .queueFamilyIndexCount {0},
            .pQueueFamilyIndices {nullptr},
        };

        const VmaAllocationCreateInfo allocationCreateInfo {
            .flags {VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT},
            .usage {VMA_MEMORY_USAGE_AUTO},
            .requiredFlags {
                static_cast<VkMemoryPropertyFlags>(memoryPropertyFlags)},
            .preferredFlags {},
            .memoryTypeBits {},
            .pool {nullptr},
            .pUserData {},
            .priority {},
        };

        VkResult result = vmaCreateBuffer(
            **this->allocator,
            &bufferCreateInfo,
            &allocationCreateInfo,
            &this->buffer,
            &this->allocation,
            nullptr);

        util::assertFatal(
            result == VK_SUCCESS,
            "Failed to allocate buffer {}",
            vk::to_string(vk::Result {result}));
    }

    Buffer::~Buffer()
    {
        this->free();
    }

    Buffer::Buffer(Buffer&& other)
        : allocator {std::move(other.allocator)}
        , buffer {other.buffer}
        , allocation {other.allocation}
        , size {other.size}
        , mapped_memory {other.mapped_memory}
    {
        other.allocator     = nullptr;
        other.buffer        = nullptr;
        other.allocation    = nullptr;
        other.size          = 0;
        other.mapped_memory = nullptr;
    }

    Buffer& Buffer::operator= (Buffer&& other)
    {
        this->free();

        this->allocator     = std::move(other.allocator);
        this->buffer        = other.buffer;
        this->allocation    = other.allocation;
        this->size          = other.size;
        this->mapped_memory = other.mapped_memory;

        other.buffer        = nullptr;
        other.allocation    = nullptr;
        other.size          = 0;
        other.mapped_memory = nullptr;

        return *this;
    }

    vk::Buffer Buffer::operator* () const
    {
        return vk::Buffer {this->buffer};
    }

    std::size_t Buffer::sizeBytes() const
    {
        return this->size;
    }

    void* Buffer::getMappedPtr() const
    {
        if (this->mapped_memory == nullptr)
        {
            VkResult result = vmaMapMemory(
                **this->allocator, this->allocation, &this->mapped_memory);

            util::assertFatal(
                result == VK_SUCCESS,
                "Failed to map buffer memory {}",
                vk::to_string(vk::Result {result}));

            util::assertFatal(
                this->mapped_memory != nullptr, "Mapped ptr was nullptr!");
        }

        return this->mapped_memory;
    }

    void Buffer::write(std::span<const std::byte> byteSpan) const
    {
        util::assertFatal(
            byteSpan.size_bytes() == this->size,
            "Tried to write {} Bytes of data to a buffer of size {}",
            byteSpan.size_bytes(),
            this->size);

        std::memcpy(
            this->getMappedPtr(), byteSpan.data(), byteSpan.size_bytes());
    }

    void
    Buffer::copyFrom(const Buffer& other, vk::CommandBuffer commandBuffer) const
    {
        util::assertFatal(
            this->size == other.size,
            "Tried to write a buffer of size {} to a buffer of size {}",
            other.size,
            this->size);

        // TODO: this should be things with the same memory pool
        // if not require the pcie extension and throw a warning because bro wtf
        util::assertFatal(
            this->allocator == other.allocator, "Allocators were not the same");

        const vk::BufferCopy bufferCopyParameters {
            .srcOffset {0},
            .dstOffset {0},
            .size {this->size},
        };

        // SRC -> DST
        commandBuffer.copyBuffer(
            other.buffer, this->buffer, bufferCopyParameters);
    }

    void Buffer::free()
    {
        if (this->allocator == nullptr)
        {
            return;
        }

        if (this->mapped_memory != nullptr)
        {
            vmaUnmapMemory(**this->allocator, this->allocation);
            this->mapped_memory = nullptr;
        }

        vmaDestroyBuffer(**this->allocator, this->buffer, this->allocation);
    }

    StagedBuffer::StagedBuffer(
        std::shared_ptr<Allocator> allocator_,
        std::size_t                sizeBytes,
        vk::BufferUsageFlags       usage)
        : allocator {std::move(allocator_)}
        , staging_buffer 
        { 
            // clang-format off
            // TODO: wtf?!
            this->allocator->shouldBufferStage()
            ?
            std::make_optional<Buffer>(
                allocator,
                sizeBytes,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible
                | vk::MemoryPropertyFlagBits::eHostCoherent
            )
            :
            std::nullopt
            // clang-format on
        }
        , gpu_local_buffer {
              std::move(allocator),
              sizeBytes,
              vk::BufferUsageFlagBits::eTransferDst | usage,
              vk::MemoryPropertyFlagBits::eDeviceLocal}
    {}

    vk::Buffer StagedBuffer::operator* () const
    {
        return *this->gpu_local_buffer;
    }

    std::size_t StagedBuffer::sizeBytes() const
    {
        return this->gpu_local_buffer.sizeBytes();
    }

    void StagedBuffer::write(std::span<const std::byte> data) const
    {
        if (this->staging_buffer.has_value())
        {
            this->staging_buffer->write(data);
        }
        else
        {
            this->gpu_local_buffer.write(data);
        }
    }

    void StagedBuffer::stage(vk::CommandBuffer commandBuffer) const
    {
        if (this->staging_buffer.has_value())
        {
            this->gpu_local_buffer.copyFrom(
                *this->staging_buffer, commandBuffer);
        }
    }

} // namespace gfx::vulkan
