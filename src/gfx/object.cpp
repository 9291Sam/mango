#include "object.hpp"
#include "vulkan/allocator.hpp"
#include "vulkan/pipeline.hpp"

namespace gfx
{
    Object::Object(
        std::shared_ptr<vulkan::Allocator> allocator_,
        std::shared_ptr<vulkan::Pipeline>  pipeline_)
        : allocator {std::move(allocator_)}
        , pipeline {std::move(pipeline_)}
    {}

    Object::~Object() {}

    std::strong_ordering
    Object::operator<=> ([[maybe_unused]] const Object& other) const
    {
        return *this->pipeline <=> *other.pipeline;
    }

    void Object::bindIfPipelineChanged(
        BindState& bindState, vk::CommandBuffer commandBuffer) const
    {
        if (bindState.current_pipeline != this->pipeline)
        {
            commandBuffer.bindPipeline(
                vk::PipelineBindPoint::eGraphics, **this->pipeline);

            bindState.current_pipeline = this->pipeline;
        }
    }

    VertexObject::VertexObject(
        std::shared_ptr<vulkan::Allocator> allocator_,
        std::shared_ptr<vulkan::Pipeline>  pipeline_,
        std::span<const vulkan::Vertex>    vertices)
        : Object {std::move(allocator_), std::move(pipeline_)}
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

    void VertexObject::bind(
        BindState& bindState, vk::CommandBuffer commandBuffer) const
    {
        this->bindIfPipelineChanged(bindState, commandBuffer);

        commandBuffer.bindVertexBuffers(0, *this->vertex_buffer, {0});
    }

    void VertexObject::draw(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.draw(
            static_cast<std::uint32_t>(this->number_of_vertices), 1, 0, 0);
    }

    IndexObject::IndexObject(
        std::shared_ptr<vulkan::Allocator> allocator_,
        std::shared_ptr<vulkan::Pipeline>  pipeline_,
        std::span<const vulkan::Vertex>    vertices,
        std::span<const vulkan::Index>     indicies)
        : VertexObject {std::move(allocator_), std::move(pipeline_), vertices}
        , number_of_indicies {indicies.size()}
        , index_buffer {
              this->allocator,
              indicies.size_bytes(),
              vk::BufferUsageFlagBits::eIndexBuffer,
              vk::MemoryPropertyFlagBits::eHostVisible
                  | vk::MemoryPropertyFlagBits::eHostCoherent
                  | vk::MemoryPropertyFlagBits::eDeviceLocal}
    {
        this->index_buffer.write(
            {reinterpret_cast<const std::byte*>(indicies.data()),
             indicies.size_bytes()});
    }

    IndexObject::~IndexObject() {}

    void IndexObject::bind(
        BindState& bindState, vk::CommandBuffer commandBuffer) const
    {
        this->bindIfPipelineChanged(bindState, commandBuffer);

        commandBuffer.bindVertexBuffers(0, *this->vertex_buffer, {0});
        commandBuffer.bindIndexBuffer(
            *this->index_buffer, 0, vk::IndexType::eUint32);
    }

    void IndexObject::draw(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.drawIndexed(
            static_cast<std::uint32_t>(this->number_of_indicies), 1, 0, 0, 0);
    }
} // namespace gfx
