#include "object.hpp"
#include "camera.hpp"
#include "vulkan/allocator.hpp"
#include "vulkan/device.hpp"
#include "vulkan/pipelines.hpp"
#include "vulkan/swapchain.hpp"
#include <map>

namespace gfx
{
    Object::Object(
        std::shared_ptr<vulkan::Device> device_,
        std::string                     name_,
        vulkan::PipelineType            pipeline,
        vulkan::DescriptorState         state)
        : name {std::move(name_)}
        , required_pipeline {pipeline}
        , required_descriptor_sets {state}
        , device {std::move(device_)}
    {}

    Object::~Object() {}

    std::strong_ordering Object::operator<=> (const Object& other) const
    {
#define EMIT_ORDERING(field)                                                   \
    if (this->field <=> other.field != std::strong_ordering::equivalent)       \
    {                                                                          \
        return this->field <=> other.field;                                    \
    }

        EMIT_ORDERING(required_pipeline)
        EMIT_ORDERING(required_descriptor_sets.descriptors[0])
        EMIT_ORDERING(required_descriptor_sets.descriptors[1])
        EMIT_ORDERING(required_descriptor_sets.descriptors[2])
        EMIT_ORDERING(required_descriptor_sets.descriptors[3])

        return std::strong_ordering::equivalent;
#undef EMIT_ORDERING
    }

    Object::operator std::string () const
    {
        return fmt::format("Object {}", this->name);
    }

    void Object::updateBindState(
        vk::CommandBuffer                                       commandBuffer,
        BindState&                                              bindState,
        const std::map<vulkan::PipelineType, vulkan::Pipeline>& pipelineMap,
        std::span<const vulkan::DescriptorSet> descriptorSets) const
    {
        if (bindState.current_pipeline != this->required_pipeline)
        {
            // util::logTrace("looking up pipeline to bind");
            commandBuffer.bindPipeline(
                vk::PipelineBindPoint::eGraphics,
                *pipelineMap.at(this->required_pipeline));

            // util::logTrace("past looking up pipeline to bind");

            bindState.current_pipeline = this->required_pipeline;

            bindState.current_descriptor_sets.reset();
        }

        for (std::size_t i = 0;
             i < bindState.current_descriptor_sets.descriptors.size();
             ++i)
        {
            vulkan::DescriptorSetType& currentSet =
                bindState.current_descriptor_sets.descriptors.at(i);
            const vulkan::DescriptorSetType requiredSet =
                this->required_descriptor_sets.descriptors.at(i);

            if (requiredSet == vulkan::DescriptorSetType::None
                || requiredSet == currentSet)
            {
                continue;
            }
            else
            {
                const vulkan::Pipeline& currentPipeline =
                    pipelineMap.at(bindState.current_pipeline);

                util::assertFatal(
                    descriptorSets.size() > i,
                    "Index {} out of bounds in span of size {}",
                    i,
                    descriptorSets.size());

                commandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    currentPipeline.getLayout(),
                    static_cast<std::uint32_t>(i),
                    std::array<vk::DescriptorSet, 1> {*descriptorSets[i]},
                    {});

                currentSet = requiredSet;
            }
        }
    }

    // clang-format doesnt work below this line TODO: what
    // clang-format on
    TriangulatedObject::TriangulatedObject(
        std::shared_ptr<vulkan::Device>    device_,
        std::shared_ptr<vulkan::Allocator> allocator_,
        vulkan::PipelineType               pipelineType,
        std::span<const vulkan::Vertex>    vertices,
        std::span<const vulkan::Index>     indices)
        // clang-format off
        : Object {
            std::move(device_),
            "Triangulated Object",
            pipelineType,
            {}
        }
        , transform {
            .translation {0.0f, 0.0f, 0.0f},
            .rotation {1.0f, 0.0f, 0.0f, 0.0f},
            .scale {1.0f, 1.0f, 1.0f}
            }
        , allocator {std::move(allocator_)}
        , number_of_vertices {vertices.size()}
        , vertex_buffer {
            this->allocator,
            vertices.size_bytes(),
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible
            | vk::MemoryPropertyFlagBits::eHostCoherent
            | vk::MemoryPropertyFlagBits::eDeviceLocal
        }
        , number_of_indices {indices.size()}
        , index_buffer {
              this->allocator,
              indices.size_bytes(),
              vk::BufferUsageFlagBits::eIndexBuffer,
              vk::MemoryPropertyFlagBits::eHostVisible
                  | vk::MemoryPropertyFlagBits::eHostCoherent
                  | vk::MemoryPropertyFlagBits::eDeviceLocal
        }
    // clang-format on
    {
        this->vertex_buffer.write(std::as_bytes(vertices));
        this->index_buffer.write(std::as_bytes(indices));
    }

    TriangulatedObject::~TriangulatedObject() {}

    void TriangulatedObject::bind(
        vk::CommandBuffer                                       commandBuffer,
        BindState&                                              bindState,
        const std::map<vulkan::PipelineType, vulkan::Pipeline>& pipelineMap)
        const
    {
        this->updateBindState(commandBuffer, bindState, pipelineMap, {});

        commandBuffer.bindVertexBuffers(0, *this->vertex_buffer, {0});
        commandBuffer.bindIndexBuffer(
            *this->index_buffer, 0, vk::IndexType::eUint32);
    }

    void TriangulatedObject::setPushConstants(
        vk::CommandBuffer       commandBuffer,
        const vulkan::Pipeline& pipeline,
        const Camera&           camera,
        vk::Extent2D            renderExtent) const
    {
        vulkan::PushConstants pushConstants {.model_view_proj {
            Camera::getPerspectiveMatrix(
                glm::radians(70.f),
                static_cast<float>(renderExtent.width)
                    / static_cast<float>(renderExtent.height),
                0.1f,
                200000.0f)
            * camera.getViewMatrix() * this->transform.asModelMatrix()}};

        commandBuffer.pushConstants<vulkan::PushConstants>(
            pipeline.getLayout(),
            vk::ShaderStageFlagBits::eVertex,
            0,
            pushConstants);
    }

    void TriangulatedObject::draw(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.drawIndexed(
            static_cast<std::uint32_t>(this->number_of_indices), 1, 0, 0, 0);
    }

    VoxelObject::VoxelObject(
        std::shared_ptr<vulkan::Device>    device_,
        std::shared_ptr<vulkan::Allocator> allocator_,
        std::span<glm::vec3>               voxelPositions)
        : Object(
            std::move(device_),
            "Voxel object",
            vulkan::PipelineType::Voxel,
            gfx::vulkan::DescriptorState {
                vulkan::DescriptorSetType::Voxel,
                vulkan::DescriptorSetType::None,
                vulkan::DescriptorSetType::None,
                vulkan::DescriptorSetType::None})
        , transform {.translation {0.0f, 0.0f, 0.0f}, .rotation {1.0f, 0.0f, 0.0f, 0.0f}, .scale {1.0f, 1.0f, 1.0f}}
        , allocator {std::move(allocator_)}
        , number_of_voxels {voxelPositions.size()}
        , positions_buffer(
              this->allocator,
              voxelPositions.size() * sizeof(glm::vec4),
              vk::BufferUsageFlagBits::eStorageBuffer)
        , sizes_buffer(
              this->allocator,
              voxelPositions.size() * sizeof(float),
              vk::BufferUsageFlagBits::eStorageBuffer)
        , colors_buffer(
              this->allocator,
              voxelPositions.size() * sizeof(glm::vec4),
              vk::BufferUsageFlagBits::eStorageBuffer)
        , voxel_set {this->allocator->allocateDescriptorSet(
              vulkan::getDescriptorSetLayout(
                  vulkan::DescriptorSetType::Voxel, this->device))}
        , have_buffers_staged {false}

    {
        // TODO: replace vec3 with vec4 in the entire engine for positions
        std::vector<glm::vec4> positionsCopy {};
        positionsCopy.reserve(voxelPositions.size());
        for (glm::vec3 v : voxelPositions)
        {
            positionsCopy.push_back(glm::vec4 {v, 1.0f});
        }

        std::vector<float> sizes {};
        sizes.resize(voxelPositions.size());
        std::ranges::fill(sizes, 10.0f);

        std::vector<glm::vec4> colors {};
        colors.resize(voxelPositions.size());
        std::ranges::fill(colors, glm::vec4 {1.0f, 1.0f, 1.0f, 1.0f});

        this->positions_buffer.write(std::as_bytes(std::span {positionsCopy}));
        this->sizes_buffer.write(std::as_bytes(std::span {sizes}));
        this->colors_buffer.write(std::as_bytes(std::span {colors}));

        const vk::DescriptorBufferInfo positionsBufferInfo {
            .buffer {*this->positions_buffer},
            .offset {0},
            .range {this->positions_buffer.sizeBytes()},
        };

        const vk::DescriptorBufferInfo sizesBufferInfo {
            .buffer {*this->sizes_buffer},
            .offset {0},
            .range {this->sizes_buffer.sizeBytes()},
        };

        const vk::DescriptorBufferInfo colorsBufferInfo {
            .buffer {*this->colors_buffer},
            .offset {0},
            .range {this->colors_buffer.sizeBytes()},
        };

        std::array<vk::WriteDescriptorSet, 3> descriptorSetWrites {
            vk::WriteDescriptorSet {
                .sType {vk::StructureType::eWriteDescriptorSet},
                .pNext {nullptr},
                .dstSet {*this->voxel_set},
                .dstBinding {0},
                .dstArrayElement {0},
                .descriptorCount {1},
                .descriptorType {vk::DescriptorType::eStorageBuffer},
                .pImageInfo {nullptr},
                .pBufferInfo {&positionsBufferInfo},
                .pTexelBufferView {nullptr},
            },
            vk::WriteDescriptorSet {
                .sType {vk::StructureType::eWriteDescriptorSet},
                .pNext {nullptr},
                .dstSet {*this->voxel_set},
                .dstBinding {1},
                .dstArrayElement {0},
                .descriptorCount {1},
                .descriptorType {vk::DescriptorType::eStorageBuffer},
                .pImageInfo {nullptr},
                .pBufferInfo {&sizesBufferInfo},
                .pTexelBufferView {nullptr},
            },
            vk::WriteDescriptorSet {
                .sType {vk::StructureType::eWriteDescriptorSet},
                .pNext {nullptr},
                .dstSet {*this->voxel_set},
                .dstBinding {2},
                .dstArrayElement {0},
                .descriptorCount {1},
                .descriptorType {vk::DescriptorType::eStorageBuffer},
                .pImageInfo {nullptr},
                .pBufferInfo {&colorsBufferInfo},
                .pTexelBufferView {nullptr},
            }};

        this->device->asLogicalDevice().updateDescriptorSets(
            descriptorSetWrites, {});
    }

    VoxelObject::~VoxelObject() {}

    void VoxelObject::bind(
        vk::CommandBuffer                                       commandBuffer,
        BindState&                                              state,
        const std::map<vulkan::PipelineType, vulkan::Pipeline>& pipelineMap)
        const
    {
        if (!this->have_buffers_staged)
        {
            this->sizes_buffer.stage(commandBuffer);
            this->colors_buffer.stage(commandBuffer);
            this->colors_buffer.stage(commandBuffer);

            this->have_buffers_staged = true;
        }

        this->updateBindState(
            commandBuffer,
            state,
            pipelineMap,
            std::span<const gfx::vulkan::DescriptorSet> {&this->voxel_set, 1});
    }

    void VoxelObject::setPushConstants(
        vk::CommandBuffer       commandBuffer,
        const vulkan::Pipeline& pipeline,
        const Camera&           camera,
        vk::Extent2D            renderExtent) const
    {
        vulkan::PushConstants pushConstants {.model_view_proj {
            Camera::getPerspectiveMatrix(
                glm::radians(70.f),
                static_cast<float>(renderExtent.width)
                    / static_cast<float>(renderExtent.height),
                0.1f,
                200000.0f)
            * camera.getViewMatrix() * this->transform.asModelMatrix()}};

        commandBuffer.pushConstants<vulkan::PushConstants>(
            pipeline.getLayout(),
            vk::ShaderStageFlagBits::eVertex,
            0,
            pushConstants);
    }

    void VoxelObject::draw(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.draw(
            static_cast<std::uint32_t>(this->number_of_voxels * 14), 1, 0, 0);
    }

    // std::size_t Object::getPipelineNumber() const
    // {
    //     return this->pipeline_number;
    // }

    // void Object::bind(vk::CommandBuffer commandBuffer) const
    // {
    //     commandBuffer.bindVertexBuffers(0, *this->vertex_buffer, {0});
    //     commandBuffer.bindIndexBuffer(
    //         *this->index_buffer, 0, vk::IndexType::eUint32);
    // }

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
