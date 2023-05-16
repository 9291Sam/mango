#ifndef SRC_GFX_VULKAN_ALLOCATOR_HPP
#define SRC_GFX_VULKAN_ALLOCATOR_HPP

#include "descriptors.hpp"
#include "includes.hpp"
#include <memory>

namespace gfx::vulkan
{
    class Instance;
    class Device;

    class Allocator
    {
    public:

        Allocator(
            std::shared_ptr<Instance>,
            std::shared_ptr<Device>,
            PFN_vkGetInstanceProcAddr,
            PFN_vkGetDeviceProcAddr);
        ~Allocator();

        Allocator(const Allocator&)             = delete;
        Allocator(Allocator&&)                  = delete;
        Allocator& operator= (const Allocator&) = delete;
        Allocator& operator= (Allocator&&)      = delete;

        bool         shouldBufferStage() const;
        VmaAllocator operator* () const;
        DescriptorSet
            allocateDescriptorSet(std::shared_ptr<DescriptorSetLayout>);


    private:
        std::shared_ptr<Instance>       instance;
        std::shared_ptr<Device>         device;
        VmaAllocator                    allocator;
        std::shared_ptr<DescriptorPool> descriptor_pool;
    };
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_ALLOCATOR_HPP
