#ifndef SRC_GFX_OBJECT_HPP
#define SRC_GFX_OBJECT_HPP

#include "transform.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/gpu_data.hpp"
#include "vulkan/includes.hpp"
#include "vulkan/pipelines.hpp"
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
        class Device;
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

        Object(
            std::shared_ptr<vulkan::Device>,
            std::string name,
            vulkan::PipelineType,
            vulkan::DescriptorState);
        virtual ~Object();

        Object(const Object&)             = delete;
        Object(Object&&)                  = delete;
        Object& operator= (const Object&) = delete;
        Object& operator= (Object&&)      = delete;

        virtual void bind(
            vk::CommandBuffer,
            BindState&,
            const std::map<vulkan::PipelineType, vulkan::Pipeline>&) const = 0;

        virtual void setPushConstants(
            vk::CommandBuffer,
            const vulkan::Pipeline&,
            const Camera&,
            vk::Extent2D renderExtent) const = 0;

        virtual void draw(vk::CommandBuffer) const = 0;

        std::strong_ordering operator<=> (const Object&) const;
        operator std::string () const;

    protected:
        void updateBindState(
            vk::CommandBuffer,
            BindState&,
            const std::map<vulkan::PipelineType, vulkan::Pipeline>&,
            std::span<const vulkan::DescriptorSet>) const;

        const std::string               name;
        const vulkan::PipelineType      required_pipeline;
        const vulkan::DescriptorState   required_descriptor_sets;
        std::shared_ptr<vulkan::Device> device;
    };

    class TriangulatedObject : public Object
    {
    public:
        TriangulatedObject(
            std::shared_ptr<vulkan::Device>,
            std::shared_ptr<vulkan::Allocator>,
            vulkan::PipelineType,
            std::span<const vulkan::Vertex>,
            std::span<const vulkan::Index>);
        ~TriangulatedObject() override;

        virtual void bind(
            vk::CommandBuffer,
            BindState&,
            const std::map<vulkan::PipelineType, vulkan::Pipeline>&)
            const override;

        virtual void setPushConstants(
            vk::CommandBuffer,
            const vulkan::Pipeline&,
            const Camera&,
            vk::Extent2D renderExtent) const override;

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

    //     VoxelObject(
    //         std::shared_ptr<vulkan::Device>,
    //         std::shared_ptr<vulkan::Allocator>,
    //         std::span<glm::vec3> voxelPositions);
    //     ~VoxelObject() override;

    //     void bind(
    //         vk::CommandBuffer,
    //         BindState&,
    //         const std::map<vulkan::PipelineType, vulkan::Pipeline>&)
    //         const override;

    //     void setPushConstants(
    //         vk::CommandBuffer,
    //         const vulkan::Pipeline&,
    //         const Camera&,
    //         vk::Extent2D renderExtent) const override;

    //     void draw(vk::CommandBuffer) const override;

    //     Transform transform;
    // private:
    //     std::shared_ptr<vulkan::Allocator> allocator;

    //     std::size_t           number_of_voxels;
    //     vulkan::StagedBuffer  positions_buffer;
    //     vulkan::StagedBuffer  sizes_buffer;
    //     vulkan::StagedBuffer  colors_buffer;
    //     vulkan::DescriptorSet voxel_set;

    //     mutable bool have_buffers_staged;
    // };

} // namespace gfx

#endif // SRC_GFX_OBJECT_HPP
