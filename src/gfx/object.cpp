#include "object.hpp"
#include "vulkan/allocator.hpp"

namespace gfx
{
    VertexObject::VertexObject(
        std::shared_ptr<vulkan::Allocator> allocator_,
        std::span<const vulkan::Vertex>    vertices)
        : allocator {std::move(allocator_)}
        , number_of_vertices {vertices.size()}
        , vertex_buffer {
              this->allocator,
              vertices.size_bytes(),
              vk::BufferUsageFlagBits::eVertexBuffer,
              vk::MemoryPropertyFlagBits::eHostVisible
                  | vk::MemoryPropertyFlagBits::eHostCoherent
                  | vk::MemoryPropertyFlagBits::eDeviceLocal}
    {
        this->vertex_buffer.write(
            {reinterpret_cast<const std::byte*>(vertices.data()),
             vertices.size_bytes()});
    }

    VertexObject::~VertexObject() {}

    void VertexObject::bind(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.bindVertexBuffers(0, *this->vertex_buffer, {0});
    }

    void VertexObject::draw(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.draw(
            static_cast<std::uint32_t>(this->number_of_vertices), 1, 0, 0);
    }
} // namespace gfx