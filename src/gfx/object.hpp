#ifndef SRC_GFX_OBJECT_HPP
#define SRC_GFX_OBJECT_HPP

#include "camera.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/gpu_data.hpp"
#include "vulkan/pipelines.hpp"
#include <optional>
#include <util/uuid.hpp>

namespace gfx
{
    class Renderer;

    namespace vulkan
    {
        class Allocator;
    }

    struct ObjectBoundDescriptor
    {
        std::optional<util::UUID> id;

        [[nodiscard]] bool operator== (const ObjectBoundDescriptor&) const;
        [[nodiscard]] std::strong_ordering
        operator<=> (const ObjectBoundDescriptor&) const;
    };

    struct BindState // NOLINT: I want designated initialization
    {
        vulkan::PipelineType                 pipeline;
        std::array<ObjectBoundDescriptor, 4> descriptors;

        [[nodiscard]] std::strong_ordering operator<=> (const BindState&) const;
    };

    class Object
    {
    public:
        Object(
            const gfx::Renderer&,
            std::string name,
            vulkan::PipelineType,
            std::array<ObjectBoundDescriptor, 4>);
        virtual ~Object() = default;

        Object(const Object&)             = delete;
        Object(Object&&)                  = delete;
        Object& operator= (const Object&) = delete;
        Object& operator= (Object&&)      = delete;

        // internal
        //
        // renderer::getPipeline(vulkan::PipelineType)
        // -> vulkan::Pipeline&
        virtual void bind(vk::CommandBuffer, BindState&) const = 0;

        // these should all be optional parameters and checked incase of
        // differing arguments
        //
        // or a variant of all the different argument types
        // get pipeline + render extent via the Self::renderer
        virtual void
        setPushConstants(vk::CommandBuffer, const Camera&) const = 0;

        virtual void draw(vk::CommandBuffer) const = 0;

        std::strong_ordering operator<=> (const Object&) const;
        explicit operator std::string () const;

        Transform transform;

    protected:
        // friendship is neither mutual nor transitive but your friends can
        // touch your privates
        // Basically we need to poke in and get some renderer internals, but we
        // dont want them fully exposed, so friending and a protected function
        // for all subclasses it is
        [[nodiscard]] std::shared_ptr<vulkan::Allocator>
                                              getRendererAllocator() const;
        [[nodiscard]] const vulkan::Pipeline& getRendererPipeline() const;

        void updateBindState(
            vk::CommandBuffer,
            BindState&,
            std::array<vk::DescriptorSet, 4> setsToBindIfRequired) const;

        const gfx::Renderer& renderer;
        const std::string    name;
        const util::UUID     id;
        const BindState      bind_state;
    };

    class SimpleTriangulatedObject final : public Object
    {
    public:
        SimpleTriangulatedObject(
            const gfx::Renderer&,
            std::span<const vulkan::Vertex>,
            std::span<const vulkan::Index>);
        ~SimpleTriangulatedObject() override = default;

        void bind(vk::CommandBuffer, BindState&) const override;
        void setPushConstants(vk::CommandBuffer, const Camera&) const override;
        void draw(vk::CommandBuffer) const override;

    private:
        std::size_t    number_of_vertices;
        vulkan::Buffer vertex_buffer;

        std::size_t    number_of_indices;
        vulkan::Buffer index_buffer;
    };

    // TODO: floating origin + everything
    //    class VoxelObject : public Object
    //    {};

} // namespace gfx

#endif // SRC_GFX_OBJECT_HPP