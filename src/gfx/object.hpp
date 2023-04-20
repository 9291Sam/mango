#ifndef SRC_GFX_OBJECT_HPP
#define SRC_GFX_OBJECT_HPP

// #include "transform.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/gpu_data.hpp"
#include "vulkan/includes.hpp"
#include <memory>
#include <span>

namespace gfx
{
    namespace vulkan
    {
        class Allocator;
    } // namespace vulkan

    class Object
    {
    public:
        Object() noexcept {}
        virtual ~Object();

        Object(const Object&)             = delete;
        Object(Object&&)                  = delete;
        Object& operator= (const Object&) = delete;
        Object& operator= (Object&&)      = delete;

        virtual void bind(vk::CommandBuffer) const = 0;
        virtual void draw(vk::CommandBuffer) const = 0;
    };

    class VertexObject : public Object
    {
    public:

        VertexObject(
            std::shared_ptr<vulkan::Allocator>,
            std::span<const vulkan::Vertex>);
        virtual ~VertexObject() override;

        virtual void bind(vk::CommandBuffer) const override;
        virtual void draw(vk::CommandBuffer) const override;

    private:
        std::shared_ptr<vulkan::Allocator> allocator;
        std::size_t                        number_of_vertices;
        vulkan::Buffer                     vertex_buffer;
    };
} // namespace gfx

#endif // SRC_GFX_OBJECT_HPP
