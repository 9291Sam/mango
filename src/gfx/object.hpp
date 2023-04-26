#ifndef SRC_GFX_OBJECT_HPP
#define SRC_GFX_OBJECT_HPP

// #include "transform.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/gpu_data.hpp"
#include "vulkan/includes.hpp"
#include <compare>
#include <memory>
#include <span>
#include <vector>

namespace gfx
{
    namespace vulkan
    {
        class Allocator;
        class Pipeline;
        class DescriptorSet;
    } // namespace vulkan

    struct BindState
    {
        std::shared_ptr<vulkan::Pipeline> current_pipeline;
    };

    class Object
    {
    public:
        virtual ~Object();

        Object(const Object&)             = delete;
        Object(Object&&)                  = delete;
        Object& operator= (const Object&) = delete;
        Object& operator= (Object&&)      = delete;

        virtual std::strong_ordering operator<=> (const Object&) const;

        virtual void bind(BindState&, vk::CommandBuffer) const = 0;
        virtual void draw(vk::CommandBuffer) const             = 0;
    protected:
        Object(
            std::shared_ptr<vulkan::Allocator>,
            std::shared_ptr<vulkan::Pipeline>);

        std::shared_ptr<vulkan::Allocator> allocator;
        std::shared_ptr<vulkan::Pipeline>  pipeline;
    };

    class VertexObject : public Object
    {
    public:
        VertexObject(
            std::shared_ptr<vulkan::Allocator>,
            std::shared_ptr<vulkan::Pipeline>,
            std::span<const vulkan::Vertex>);
        virtual ~VertexObject() override;

        virtual void bind(BindState&, vk::CommandBuffer) const override;
        virtual void draw(vk::CommandBuffer) const override;

    private:
        std::size_t    number_of_vertices;
        vulkan::Buffer vertex_buffer;
    };

    // class IndexObject : public VertexObject
    // {
    // public:
    // IndexObject(
    // std::shared_ptr<vulkan::Allocator>,
    // std::shared_ptr<vulkan::Pipeline>,
    // std::span<const vulkan::Vertex>,
    // std::span<const vulkan::Index>);
    // virtual IndexObject()
    // }

    // std::vector<vulkan::DescriptorSet>
    //   descriptors;
    // std::size_t                   number_of_indicies;
    // std::optional<vulkan::Buffer> index_buffer;

} // namespace gfx

#endif // SRC_GFX_OBJECT_HPP
