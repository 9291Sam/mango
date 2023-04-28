#include "object.hpp"
#include "camera.hpp"
#include "vulkan/allocator.hpp"
#include "vulkan/pipeline.hpp"
#include "vulkan/swapchain.hpp"

namespace gfx
{
    Object::Object(
        std::shared_ptr<vulkan::Allocator> allocator_,
        std::shared_ptr<vulkan::Pipeline>  pipeline_,
        std::shared_ptr<vulkan::Swapchain> swapchain_)
        : transform {.translation {0.0f, 0.0f, 0.0f}, .rotation {1.0f, 0.0f, 0.0f, 0.0f}, .scale {1.0f, 1.0f, 1.0f}}
        , allocator {std::move(allocator_)}
        , pipeline {std::move(pipeline_)}
        , swapchain {std::move(swapchain_)}

    {}

    Object::~Object() {}

    std::strong_ordering Object::operator<=> (const Object& other) const
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

    void Object::setPushConstants(
        const Camera& camera, vk::CommandBuffer commandBuffer) const
    {
        this->transform.assertReasonable();

        vulkan::PushConstants pushConstants {.model_view_proj {
            Camera::getPerspectiveMatrix(
                glm::radians(70.f),
                static_cast<float>(this->swapchain->getExtent().width)
                    / static_cast<float>(this->swapchain->getExtent().height),
                0.1f,
                200000.0f)
            * camera.getViewMatrix() * this->transform.asModelMatrix()}};

        commandBuffer.pushConstants<vulkan::PushConstants>(
            this->pipeline->getLayout(),
            vk::ShaderStageFlagBits::eAllGraphics,
            0,
            pushConstants);
    }

    VertexObject::VertexObject(
        std::shared_ptr<vulkan::Allocator> allocator_,
        std::shared_ptr<vulkan::Pipeline>  pipeline_,
        std::shared_ptr<vulkan::Swapchain> swapchain_,
        std::span<const vulkan::Vertex>    vertices)
        : Object {std::move(allocator_), std::move(pipeline_), std::move(swapchain_)}
        , number_of_vertices {vertices.size()}
        , vertex_buffer {
              this->allocator,
              vertices.size_bytes(),
              vk::BufferUsageFlagBits::eVertexBuffer,
              vk::MemoryPropertyFlagBits::eHostVisible
                  | vk::MemoryPropertyFlagBits::eHostCoherent
                  | vk::MemoryPropertyFlagBits::eDeviceLocal}
    {
        this->vertex_buffer.write(std::as_bytes(vertices));
    }

    VertexObject::~VertexObject() {}

    void VertexObject::bind(
        BindState& bindState, vk::CommandBuffer commandBuffer) const
    {
        this->bindIfPipelineChanged(bindState, commandBuffer);

        commandBuffer.bindVertexBuffers(0, *this->vertex_buffer, {0});
    }

    void VertexObject::draw(
        const Camera& camera, vk::CommandBuffer commandBuffer) const
    {
        this->setPushConstants(camera, commandBuffer);

        commandBuffer.draw(
            static_cast<std::uint32_t>(this->number_of_vertices), 1, 0, 0);
    }

    IndexObject::IndexObject(
        std::shared_ptr<vulkan::Allocator> allocator_,
        std::shared_ptr<vulkan::Pipeline>  pipeline_,
        std::shared_ptr<vulkan::Swapchain> swapchain_,
        std::span<const vulkan::Vertex>    vertices,
        std::span<const vulkan::Index>     indicies)
        : VertexObject {std::move(allocator_), std::move(pipeline_), std::move(swapchain_), vertices}
        , number_of_indicies {indicies.size()}
        , index_buffer {
              this->allocator,
              indicies.size_bytes(),
              vk::BufferUsageFlagBits::eIndexBuffer,
              vk::MemoryPropertyFlagBits::eHostVisible
                  | vk::MemoryPropertyFlagBits::eHostCoherent
                  | vk::MemoryPropertyFlagBits::eDeviceLocal}
    {
        this->index_buffer.write(std::as_bytes(indicies));
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

    void IndexObject::draw(
        const Camera& camera, vk::CommandBuffer commandBuffer) const
    {
        this->setPushConstants(camera, commandBuffer);

        commandBuffer.drawIndexed(
            static_cast<std::uint32_t>(this->number_of_indicies), 1, 0, 0, 0);
    }
} // namespace gfx
