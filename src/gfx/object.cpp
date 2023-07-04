#include "object.hpp"
#include "renderer.hpp"
#include "vulkan/allocator.hpp"

bool gfx::ObjectBoundDescriptor::operator== (
    const gfx::ObjectBoundDescriptor& other) const
{
    return this->id == other.id;
}
std::strong_ordering gfx::ObjectBoundDescriptor::operator<=> (
    const gfx::ObjectBoundDescriptor& other) const
{
    return this->id <=> other.id;
}

std::strong_ordering gfx::BindState::operator<=> (const BindState& other) const
{
#define EMIT_ORDERING(field)                                                   \
    if (this->field <=> other.field != std::strong_ordering::equivalent)       \
    {                                                                          \
        return this->field <=> other.field;                                    \
    }

    EMIT_ORDERING(pipeline)       // NOLINT: shut the fuck up
    EMIT_ORDERING(descriptors[0]) // NOLINT: shut the fuck up
    EMIT_ORDERING(descriptors[1]) // NOLINT: shut the fuck up
    EMIT_ORDERING(descriptors[2]) // NOLINT: shut the fuck up
    EMIT_ORDERING(descriptors[3]) // NOLINT: shut the fuck up

    return std::strong_ordering::equivalent;
#undef EMIT_ORDERING
}

gfx::Object::Object(
    const gfx::Renderer&                 renderer_,
    std::string                          name_,
    vulkan::PipelineType                 pipeline_,
    std::array<ObjectBoundDescriptor, 4> descriptors_)
    : renderer {renderer_}
    , name {std::move(name_)}
    , id {}
    , bind_state {.pipeline {pipeline_}, .descriptors {descriptors_}}
    , transform {
          .translation {0.0f, 0.0f, 0.0f},
          .rotation {1.0f, 0.0f, 0.0f, 0.0f},
          .scale {1.0f, 1.0f, 1.0f}}
{
    util::logTrace("Constructed Object | {}", static_cast<std::string>(*this));
}

std::strong_ordering gfx::Object::operator<=> (const Object& other) const
{
    return this->bind_state <=> other.bind_state;
}

gfx::Object::operator std::string () const
{
    return fmt::format(
        "Object {} | ID: {}", this->name, static_cast<std::string>(this->id));
}

std::shared_ptr<gfx::vulkan::Allocator>
gfx::Object::getRendererAllocator() const
{
    return this->renderer.allocator;
}

const gfx::vulkan::Pipeline& gfx::Object::getRendererPipeline() const
{
    return this->renderer.pipeline_map.at(this->bind_state.pipeline);
}

void gfx::Object::updateBindState(
    vk::CommandBuffer                commandBuffer,
    gfx::BindState&                  bindState,
    std::array<vk::DescriptorSet, 4> setsToBindIfRequired) const
{
    const vulkan::Pipeline& requiredPipeline =
        this->renderer.pipeline_map.at(this->bind_state.pipeline);

    if (bindState.pipeline != this->bind_state.pipeline)
    {
        bindState.pipeline = this->bind_state.pipeline;

        commandBuffer.bindPipeline(
            vk::PipelineBindPoint::eGraphics, *requiredPipeline);

        // Binding a new pipeline also resets descriptor sets
        bindState.descriptors = {
            std::nullopt, std::nullopt, std::nullopt, std::nullopt};
    }

    for (std::size_t idx = 0; idx < bindState.descriptors.size(); ++idx)
    {
        // TODO: optimization for the currently bound but nullopt case, will be
        // important later
        if (bindState.descriptors[idx] != this->bind_state.descriptors[idx])
        {
            bindState.descriptors[idx] = this->bind_state.descriptors[idx];

            util::assertFatal(
                setsToBindIfRequired[idx] != vk::DescriptorSet {nullptr},
                "Bind of nullptr set was requested!");

            commandBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                requiredPipeline.getLayout(),
                static_cast<std::uint32_t>(idx),
                setsToBindIfRequired[idx],
                {});
        }
    }
}

gfx::SimpleTriangulatedObject::SimpleTriangulatedObject(
    const gfx::Renderer&      renderer_,
    std::span<const vulkan::Vertex> vertices,
    std::span<const vulkan::Index>  indices)
    : Object {
        renderer_,
        fmt::format(
            "SimpleTriangulatedObject | Vertices: {} | Indices: {}",
            vertices.size(),
            indices.size()),
        vulkan::PipelineType::Flat,
        std::array<gfx::ObjectBoundDescriptor, 4> {
            ObjectBoundDescriptor {std::nullopt},
            ObjectBoundDescriptor {std::nullopt},
            ObjectBoundDescriptor {std::nullopt},
            ObjectBoundDescriptor {std::nullopt}}}
    , number_of_vertices {vertices.size()}
    , vertex_buffer {this->getRendererAllocator(),
                     vertices.size_bytes(),
                     vk::BufferUsageFlagBits::eVertexBuffer,
                     vk::MemoryPropertyFlagBits::eHostVisible
                         | vk::MemoryPropertyFlagBits::eHostCoherent
                         | vk::MemoryPropertyFlagBits::eDeviceLocal}
    , number_of_indices {indices.size()}
    , index_buffer {this->getRendererAllocator(),
                    indices.size_bytes(),
                    vk::BufferUsageFlagBits::eIndexBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible
                        | vk::MemoryPropertyFlagBits::eHostCoherent
                        | vk::MemoryPropertyFlagBits::eDeviceLocal}
{
    this->vertex_buffer.write(std::as_bytes(vertices));
    this->index_buffer.write(std::as_bytes(indices));
}

void gfx::SimpleTriangulatedObject::bind(
    vk::CommandBuffer commandBuffer, gfx::BindState& bindState) const
{
    this->updateBindState(
        commandBuffer, bindState, {nullptr, nullptr, nullptr, nullptr});

    commandBuffer.bindVertexBuffers(0, *this->vertex_buffer, {0});

    static_assert(sizeof(std::uint32_t) == sizeof(vulkan::Index));

    commandBuffer.bindIndexBuffer(
        *this->index_buffer, 0, vk::IndexType::eUint32);
}

void gfx::SimpleTriangulatedObject::setPushConstants(
    vk::CommandBuffer commandBuffer, const gfx::Camera& camera) const
{
    vulkan::PushConstants pushConstants {.model_view_proj {
        Camera::getPerspectiveMatrix(
            this->renderer.getFovYRadians(),
            static_cast<float>(this->renderer.getExtent().width)
                / static_cast<float>(this->renderer.getExtent().height),
            0.1f,
            200000.0f)
        * camera.getViewMatrix() * this->transform.asModelMatrix()}};

    commandBuffer.pushConstants<vulkan::PushConstants>(
        this->getRendererPipeline().getLayout(),
        vk::ShaderStageFlagBits::eVertex,
        0,
        pushConstants);
}

void gfx::SimpleTriangulatedObject::draw(vk::CommandBuffer commandBuffer) const
{
    commandBuffer.drawIndexed(
        static_cast<std::uint32_t>(this->number_of_indices), 1, 0, 0, 0);
}
