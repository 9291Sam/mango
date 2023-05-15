#ifndef SRC_GFX_OBJECT_HPP
#define SRC_GFX_OBJECT_HPP

#include "transform.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/gpu_data.hpp"
#include "vulkan/includes.hpp"
#include "vulkan/pipeline.hpp"
#include <compare>
#include <map>
#include <memory>
#include <ranges>
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
        BindState()
            : current_pipeline {vulkan::PipelineType::None}
            , current_descriptor_sets {}
        {}

        vulkan::PipelineType    current_pipeline;
        vulkan::DescriptorState current_descriptor_sets;
    };

    class Object
    {
    public:

        Object(std::string name, vulkan::PipelineType, vulkan::DescriptorState);
        virtual ~Object();

        Object(const Object&)             = delete;
        Object(Object&&)                  = delete;
        Object& operator= (const Object&) = delete;
        Object& operator= (Object&&)      = delete;

        virtual void bind(
            vk::CommandBuffer,
            BindState&,
            const std::map<
                vulkan::PipelineType,
                std::unique_ptr<vulkan::Pipeline>>&) const = 0;
        virtual void draw(vk::CommandBuffer) const         = 0;

        std::strong_ordering operator<=> (const Object&) const;
        operator std::string () const;

    protected:
        void updateBindState(
            vk::CommandBuffer,
            BindState&,
            const std::
                map<vulkan::PipelineType, std::unique_ptr<vulkan::Pipeline>>&,
            std::span<vulkan::DescriptorSet>) const;

        const std::string             name;
        const vulkan::PipelineType    required_pipeline;
        const vulkan::DescriptorState required_descriptor_sets;
    };

    class TriangulatedObject : public Object
    {
    public:
        TriangulatedObject(
            std::shared_ptr<vulkan::Allocator>,
            vulkan::PipelineType,
            std::span<const vulkan::Vertex>,
            std::span<const vulkan::Index>);
        ~TriangulatedObject() = default;

        TriangulatedObject(const TriangulatedObject&)             = delete;
        TriangulatedObject(TriangulatedObject&&)                  = delete;
        TriangulatedObject& operator= (const TriangulatedObject&) = delete;
        TriangulatedObject& operator= (TriangulatedObject&&)      = delete;

        virtual void bind(
            vk::CommandBuffer,
            BindState&,
            const std::map<
                vulkan::PipelineType,
                std::unique_ptr<vulkan::Pipeline>>&) const override;
        virtual void draw(vk::CommandBuffer) const override;

        Transform transform;
    private:

        std::shared_ptr<vulkan::Allocator> allocator;

        std::size_t    number_of_vertices;
        vulkan::Buffer vertex_buffer;

        std::size_t    number_of_indices;
        vulkan::Buffer index_buffer;
    };

    // class VoxelObject : public Object
    // {
    // public:

    //     // owns its own descriptors!
    // }

} // namespace gfx

#endif // SRC_GFX_OBJECT_HPP
