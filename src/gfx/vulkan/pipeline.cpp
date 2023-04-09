#include "pipeline.hpp"
#include "util/log.hpp"

namespace gfx::vulkan
{
    vk::UniqueShaderModule Pipeline::createShaderModule(
        vk::Device device, std::span<const std::byte> data
    )
    {
        util::assertFatal(data.size_bytes() % 4 == 0, "Data was not % 4");

        return device.createShaderModuleUnique(vk::ShaderModuleCreateInfo {
            .sType {vk::StructureType::eShaderModuleCreateInfo},
            .pNext {nullptr},
            .flags {},
            .codeSize {data.size_bytes()},
            .pCode {reinterpret_cast<const uint32_t*>(data.data())},
            // the default allocator of std::vector ensures this is fine
        });
    }
} // namespace gfx::vulkan