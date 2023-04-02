#ifndef SRC_GFX_VULKAN_INSTANCE_HPP
#define SRC_GFX_VULKAN_INSTANCE_HPP

#include "includes.hpp"
#include <functional>
#include <utility>

namespace gfx::vulkan
{
    /// @brief Abstraction over a vulkan instance with debug layers
    /// conditionally enabled by the environment variable
    /// MANGO_ENABLE_VULKAN_VALIDATION
    class Instance
    {
    public:
        Instance(
            PFN_vkGetInstanceProcAddr,
            std::function<void(vk::Instance)> dynamicLoaderInitalizationCallback
        );
        ~Instance();

        Instance(const Instance&)             = delete;
        Instance(Instance&&)                  = delete;
        Instance& operator= (const Instance&) = delete;
        Instance& operator= (Instance&&)      = delete;

        [[nodiscard]] vk::Instance operator* () const;

    private:
        vk::UniqueInstance        instance;
        PFN_vkGetInstanceProcAddr dyn_vk_get_instance_proc_addr;
        VkDebugUtilsMessengerEXT  debug_messenger;
        std::uint32_t             vulkan_api_version;
    };

} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_INSTANCE_HPP
