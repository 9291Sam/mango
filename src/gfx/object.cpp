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
        util::todo();
        // std::strong_ordering pipelineOrder =
        //     std::strong_order(*this->pipeline, *other.pipeline);

        // if (pipelineOrder != std::strong_order::equivalent)
        // {
        //     return pipelineOrder;
        // }

        // return pipelineOrder;
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
        if (bindState.current_pipeline != this->pipeline)
        {
            commandBuffer.bindPipeline(
                vk::PipelineBindPoint::eGraphics, **this->pipeline);
        }

        commandBuffer.bindVertexBuffers(0, *this->vertex_buffer, {0});
    }

    void VertexObject::draw(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.draw(
            static_cast<std::uint32_t>(this->number_of_vertices), 1, 0, 0);
    }
} // namespace gfx
