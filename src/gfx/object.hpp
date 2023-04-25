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

    class BindState
    {};

    class Object
    {
    public:
        Object(
            std::shared_ptr<vulkan::Allocator>,
            std::shared_ptr<vulkan::Pipeline>);
        virtual ~Object();

        Object(const Object&)             = delete;
        Object(Object&&)                  = delete;
        Object& operator= (const Object&) = delete;
        Object& operator= (Object&&)      = delete;

        virtual std::strong_ordering operator<=> (const Object&) const;

        virtual void bind(BindState&) const = 0;
        virtual void draw() const           = 0;
    protected:
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
        virtual ~VertexObject();

        virtual std::strong_ordering operator<=> (const Object&) const;

        virtual void bind(BindState&) const;
        virtual void draw() const;

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
