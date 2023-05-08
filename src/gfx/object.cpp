#include "object.hpp"
#include "camera.hpp"
#include "vulkan/allocator.hpp"
#include "vulkan/pipeline.hpp"
#include "vulkan/swapchain.hpp"

namespace gfx
{
    Object::Object(
        std::shared_ptr<vulkan::Allocator> allocator_,
        std::size_t                        pipelineNumber,
        std::span<const vulkan::Vertex>    vertices,
        std::span<const vulkan::Index>     indices)
        : transform { // clang-format off
            .translation {0.0f, 0.0f, 0.0f},
            .rotation {1.0f, 0.0f, 0.0f, 0.0f},
            .scale {1.0f, 1.0f, 1.0f}
        } // clang-format on
        , allocator {std::move(allocator_)}
        , pipeline_number {pipelineNumber}
        , number_of_vertices {vertices.size()}
        , vertex_buffer {this->allocator,
              vertices.size_bytes(),
              vk::BufferUsageFlagBits::eVertexBuffer,
              vk::MemoryPropertyFlagBits::eHostVisible
                  | vk::MemoryPropertyFlagBits::eHostCoherent
                  | vk::MemoryPropertyFlagBits::eDeviceLocal}
        , number_of_indices {indices.size()}
        , index_buffer {this->allocator,
              indices.size_bytes(),
              vk::BufferUsageFlagBits::eIndexBuffer,
              vk::MemoryPropertyFlagBits::eHostVisible
                  | vk::MemoryPropertyFlagBits::eHostCoherent
                  | vk::MemoryPropertyFlagBits::eDeviceLocal}
    {
        this->vertex_buffer.write(std::as_bytes(vertices));
        this->index_buffer.write(std::as_bytes(indices));
    }

    std::size_t Object::getPipelineNumber() const
    {
        return this->pipeline_number;
    }

    void Object::bind(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.bindVertexBuffers(0, *this->vertex_buffer, {0});
        commandBuffer.bindIndexBuffer(
            *this->index_buffer, 0, vk::IndexType::eUint32);
    }

    void Object::draw(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.drawIndexed(
            static_cast<std::uint32_t>(this->number_of_indices), 1, 0, 0, 0);
    }

    // std::strong_ordering Object::operator<=> (const Object& other) const
    // {
    //     return *this->pipeline <=> *other.pipeline;
    // }

    // void Object::bindIfPipelineChanged(
    //     BindState& bindState, vk::CommandBuffer commandBuffer) const
    // {
    //     if (bindState.current_pipeline != this->pipeline)
    //     {

    //         bindState.current_pipeline = this->pipeline;
    //     }
    // }

    // void Object::setPushConstants(
    //     const Camera&     camera,
    //     vk::Extent2D      size,
    //     vk::CommandBuffer commandBuffer) const
    // {
    //     this->transform.assertReasonable();

    //     vulkan::PushConstants pushConstants {.model_view_proj {
    //         Camera::getPerspectiveMatrix(
    //             glm::radians(70.f),
    //             static_cast<float>(size.width)
    //                 / static_cast<float>(size.height),
    //             0.1f,
    //             200000.0f)
    //         * camera.getViewMatrix() * this->transform.asModelMatrix()}};

    //     commandBuffer.pushConstants<vulkan::PushConstants>(
    //         this->pipeline->getLayout(),
    //         vk::ShaderStageFlagBits::eAllGraphics,
    //         0,
    //         pushConstants);
    // }

    // VertexObject::VertexObject(
    //     std::shared_ptr<vulkan::Allocator> allocator_,
    //     std::shared_ptr<vulkan::Pipeline>  pipeline_,
    //     std::shared_ptr<vulkan::Swapchain> swapchain_,
    //     std::span<const vulkan::Vertex>    vertices)
    //     : Object {std::move(allocator_), std::move(pipeline_),
    //     std::move(swapchain_)} , number_of_vertices {vertices.size()} ,
    //     vertex_buffer {
    //           this->allocator,
    //           vertices.size_bytes(),
    //           vk::BufferUsageFlagBits::eVertexBuffer,
    //           vk::MemoryPropertyFlagBits::eHostVisible
    //               | vk::MemoryPropertyFlagBits::eHostCoherent
    //               | vk::MemoryPropertyFlagBits::eDeviceLocal}
    // {}

    // VertexObject::~VertexObject() {}

    // void VertexObject::bind(
    //     BindState& bindState, vk::CommandBuffer commandBuffer) const
    // {
    //     this->bindIfPipelineChanged(bindState, commandBuffer);

    //     commandBuffer.bindVertexBuffers(0, *this->vertex_buffer, {0});
    // }

    // void VertexObject::draw(
    //     const Camera& camera, vk::CommandBuffer commandBuffer) const
    // {
    //     this->setPushConstants(camera, commandBuffer);

    //     commandBuffer.draw(
    //         static_cast<std::uint32_t>(this->number_of_vertices), 1, 0, 0);
    // }

    // IndexObject::IndexObject(
    //     std::shared_ptr<vulkan::Allocator> allocator_,
    //     std::shared_ptr<vulkan::Pipeline>  pipeline_,
    //     std::shared_ptr<vulkan::Swapchain> swapchain_,
    //     std::span<const vulkan::Vertex>    vertices,
    //     std::span<const vulkan::Index>     indices)
    //     : VertexObject {std::move(allocator_), std::move(pipeline_),
    //     std::move(swapchain_), vertices} , number_of_indices
    //     {indices.size()} , index_buffer {}
    // {}

    // IndexObject::~IndexObject() {}

    // void IndexObject::bind(
    //     BindState& bindState, vk::CommandBuffer commandBuffer) const
    // {
    //     this->bindIfPipelineChanged(bindState, commandBuffer);
    // }

    // void IndexObject::draw(
    //     const Camera& camera, vk::CommandBuffer commandBuffer) const
    // {
    //     this->setPushConstants(camera, commandBuffer);
    // }
} // namespace gfx
