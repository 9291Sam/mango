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

} // namespace gfx
