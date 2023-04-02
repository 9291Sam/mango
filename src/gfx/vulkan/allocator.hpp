#ifndef SRC_GFX_VULKAN_ALLOCATOR_HPP
#define SRC_GFX_VULKAN_ALLOCATOR_HPP

#include "includes.hpp"

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
            PFN_vkGetDeviceProcAddr
        );
        ~Allocator();

        Allocator(const Allocator&)             = delete;
        Allocator(Allocator&&)                  = delete;
        Allocator& operator= (const Allocator&) = delete;
        Allocator& operator= (Allocator&&)      = delete;

        [[nodiscard]] VmaAllocator operator* () const;

    private:
        std::shared_ptr<Instance> instance;
        std::shared_ptr<Device>   device;
        VmaAllocator              allocator;
    };
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_ALLOCATOR_HPP
