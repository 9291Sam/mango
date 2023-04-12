#ifndef SRC_GFX_VULKAN_PIPELINE_HPP
#define SRC_GFX_VULKAN_PIPELINE_HPP

#include "includes.hpp"
#include "util/log.hpp"
#include <memory>
#include <span>
#include <type_traits>

namespace gfx::vulkan
{
    // TODO: replace with a builder!
    class Device;
    class RenderPass;
    class Swapchain;
    class DescriptorPool; // TODO: do this first

    // class PipelineBase
    // {
    // public:
    //     static vk::UniqueShaderModule
    //     createShaderModule(vk::Device device, std::span<const std::byte>
    //     data)
    //     {
    //         util::assertFatal(data.size_bytes() % 4 == 0, "Data was not %
    //         4");

    //         return device.createShaderModuleUnique(vk::ShaderModuleCreateInfo
    //         {
    //             .sType {vk::StructureType::eShaderModuleCreateInfo},
    //             .pNext {nullptr},
    //             .flags {},
    //             .codeSize {data.size_bytes()},
    //             .pCode {reinterpret_cast<const uint32_t*>(data.data())},
    //             // the default allocator of std::vector ensures this is fine
    //         });
    //     }
    // public:

    //     PipelineBase(
    //         std::shared_ptr<Device>         device_,
    //         std::shared_ptr<RenderPass>     renderPass,
    //         std::shared_ptr<Swapchain>      swapchain_,
    //         std::shared_ptr<DescriptorPool> descriptorPool
    //     )
    //         : device {std::move(device_)}
    //         , render_pass {std::move(renderPass)}
    //         , swapchain {std::move(swapchain_)}
    //         , descriptor_pool {std::move(descriptorPool)}
    //     {}
    //     ~PipelineBase() = default;

    //     void bindPipeline(vk::CommandBuffer buffer, vk::PipelineBindPoint
    //     point)
    //     {
    //         buffer.bindPipeline(point, *this->pipeline);
    //     }

    // private:
    //     std::shared_ptr<Device>         device;
    //     std::shared_ptr<RenderPass>     render_pass;
    //     std::shared_ptr<Swapchain>      swapchain;
    //     std::shared_ptr<DescriptorPool> descriptor_pool;
    // }; // class PipelineBase

    // class FlatPipeline : public PipelineBase<FlatPipeline>
    // {
    //     FlatPipeline();
    // };
    // static_assert(ValidPipeline<FlatPipeline>);

    // // TODO: auto sorting based on hierarcicail pipeline layouts

    // /// pipelines.sort()
    // ///
    // /// for each [pipeline, object] enumerate
    // /// {
    // ///  bindDifferences(previousPipeline, currentPipelinex)
    // ///
    // /// }
    ///

} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_PIPELINE_HPP
