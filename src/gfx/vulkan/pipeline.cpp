#include "pipeline.hpp"

namespace gfx::vulkan
{
    vk::UniqueShaderModule Pipeline::createShaderModule(
        vk::Device device, std::span<const std::byte> data
    )
    {
        util::assertFatal("");

        return device.createShaderModuleUnique(vk::ShaderModuleCreateInfo {
            .sType {vk::StructureType::eShaderModuleCreateInfo},
            .pNext {nullptr},
            .flags {},
            .codeSize {spirvBytes.size_bytes()},
            .pCode {reinterpret_cast<const uint32_t*>(spirvBytes.data())},
            // the default allocator of std::vector ensures this is fine
        })
    }
} // namespace gfx::vulkan