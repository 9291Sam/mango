#ifndef SRC_GFX_OBJECT_HPP
#define SRC_GFX_OBJECT_HPP

#include "transform.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/gpu_data.hpp"
#include "vulkan/includes.hpp"
#include <compare>
#include <memory>
#include <span>
#include <vector>

namespace gfx
{
    class Camera;

    namespace vulkan
    {
        class Allocator;
        class Pipeline;
        class DescriptorSet;
        class Swapchain;
    } // namespace vulkan

    struct BindState
    {
        std::shared_ptr<vulkan::Pipeline> current_pipeline;
    };

    class Object
    {
    public:
        Object() = default;
        Object(
            std::shared_ptr<vulkan::Allocator>,
            std::size_t pipelineNumber,
            std::span<const vulkan::Vertex>,
            std::span<const vulkan::Index>);
        ~Object() = default;

        Object(const Object&)             = delete;
        Object(Object&&)                  = default;
        Object& operator= (const Object&) = delete;
        Object& operator= (Object&&)      = default;

        std::size_t getPipelineNumber() const;

        void bind(vk::CommandBuffer) const;
        void draw(vk::CommandBuffer) const;

        Transform transform;
    private:

        std::shared_ptr<vulkan::Allocator> allocator;
        std::size_t                        pipeline_number;

        std::size_t    number_of_vertices;
        vulkan::Buffer vertex_buffer;

        std::size_t    number_of_indices;
        vulkan::Buffer index_buffer;
    };

} // namespace gfx

#endif // SRC_GFX_OBJECT_HPP
