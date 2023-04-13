#include "pipeline.hpp"
#include "util/log.hpp"
#include <fstream>

namespace gfx::vulkan
{
    vk::UniqueShaderModule
    Pipeline::createShaderFromFile(vk::Device device, const char* filePath)
    {
        std::ifstream fileStream {
            filePath, std::ios::in | std::ios::binary | std::ios::ate};

        util::assertFatal(
            fileStream.is_open(), "Failed to open file |{}|", filePath);

        std::vector<char> charBuffer {};
        charBuffer.reserve(static_cast<std::size_t>(fileStream.tellg()));

        fileStream.seekg(0);
        fileStream.read(
            charBuffer.data(), static_cast<std::streamsize>(charBuffer.size()));

        // TODO: intellectual exercise, remove this UB.
        // use aligned alloc to make new array
        // copy bitwise
        // launder
        // copy form u32 array into proper vector
        vk::ShaderModuleCreateInfo shaderCreateInfo {
            .sType {vk::StructureType::eShaderModuleCreateInfo},
            .pNext {nullptr},
            .flags {},
            .codeSize {charBuffer.size()},
            .pCode {reinterpret_cast<const uint32_t*>(charBuffer.data())},
        };

        return device.createShaderModuleUnique(shaderCreateInfo);
    }

    Pipeline::Pipeline(
        std::shared_ptr<Device>                            device_,
        std::shared_ptr<Swapchain>                         swapchain_,
        std::shared_ptr<RenderPass>                        renderPass,
        std::shared_ptr<DescriptorPool>                    descriptorPool,
        std::span<const vk::PipelineShaderStageCreateInfo> shaderStages,
        vk::UniquePipelineLayout                           layout_)
        : device {std::move(device_)}
        , swapchain {std::move(swapchain_)}
        , render_pass {std::move(renderPass)}
        , descriptor_pool {std::move(descriptorPool)}
        , layout {std::move(layout_)}
        , pipeline {nullptr}
    {
        std::ignore = shaderStages;
        util::assertFatal(false, "unimplemented");
    }
} // namespace gfx::vulkan
