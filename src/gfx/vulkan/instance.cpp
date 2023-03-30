#include "instance.hpp"
#include "GLFW/glfw3.h"
#include "util/log.hpp"
#include <iostream>

constexpr bool ENABLE_VALIDATION_LAYERS = true;

static VkBool32 debugMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    [[maybe_unused]] void                      *pUserData
)
{
    util::panic(
        "Validation Layer Message: Severity: {} | Type: {} | \n{}",
        messageSeverity,
        messageType,
        pCallbackData->pMessage
    );

    return VK_FALSE;
}

// The below two functions vk{Create, Destroy}DebugUtilsMessengerEXT must by
// aren't loaded yet. Only vkGetInstanceProcAddr so we have to bootstrap
// the instance so it can init everything else
[[nodiscard]] static auto dynVkCreateDebugUtilsMessengerEXT(
    PFN_vkGetInstanceProcAddr                 dynVkGetInstanceProcAddr,
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks              *pAllocator,
    VkDebugUtilsMessengerEXT                 *pMessenger
) -> VkResult
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-strict"
    const auto maybeVkCreateDebugUtilsMessengerExt =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            dynVkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
        );
#pragma clang diagnostic pop

    util::assertFatal(
        maybeVkCreateDebugUtilsMessengerExt != nullptr,
        "Failed to dyn load vkCreateDebugUtilsMessengerEXT!"
    );

    return maybeVkCreateDebugUtilsMessengerExt(
        instance, pCreateInfo, pAllocator, pMessenger
    );
}

static void dynVkDestroyDebugUtilsMessengerEXT(
    PFN_vkGetInstanceProcAddr                     dynVkGetInstanceProcAddr,
    VkInstance                                    instance,
    VkDebugUtilsMessengerEXT                      messenger,
    [[maybe_unused]] const VkAllocationCallbacks *pAllocator
)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-strict"
    const auto maybeVkDestroyDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            dynVkGetInstanceProcAddr(
                instance, "vkDestroyDebugUtilsMessengerEXT"
            )
        );
#pragma clang diagnostic pop

    util::assertFatal(
        maybeVkDestroyDebugUtilsMessengerEXT != nullptr,
        "Failed to dy load vkDestroyDebugUtilsMessengerEXT!"
    );

    // Function always succeeds
    maybeVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

namespace gfx::vulkan
{
    Instance::Instance(
        PFN_vkGetInstanceProcAddr         dyn_vk_get_instance_proc_addr_,
        std::function<void(vk::Instance)> dynamicLoaderInitalizationCallback
    )
        : instance {nullptr}
        , dyn_vk_get_instance_proc_addr {dyn_vk_get_instance_proc_addr_}
        , debug_messenger {nullptr}
        , vulkan_api_version {VK_API_VERSION_1_3}
    {
        const vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo {
            .sType {vk::StructureType::eDebugUtilsMessengerCreateInfoEXT},
            .pNext {nullptr},
            .flags {},
            .messageSeverity {
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError},
            .messageType {
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance},
            .pfnUserCallback {debugMessageCallback},
            .pUserData {},
        };

        const vk::ApplicationInfo applicationInfo {
            .sType {vk::StructureType::eApplicationInfo},
            .pNext {nullptr},
            .pApplicationName {"mango"},
            .applicationVersion {VK_MAKE_API_VERSION(
                VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TWEAK
            )},
            .pEngineName {"mango"},
            .engineVersion {VK_MAKE_API_VERSION(
                VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TWEAK
            )},
            .apiVersion {this->vulkan_api_version},
        };

        const std::vector<const char *> instanceLayers = []
        {
            if constexpr (ENABLE_VALIDATION_LAYERS)
            {
                for (vk::LayerProperties layer :
                     vk::enumerateInstanceLayerProperties())
                {
                    if (std::strcmp(
                            layer.layerName, "VK_LAYER_KHRONOS_validation"
                        )
                        == 0)
                    {
                        return std::vector<const char *> {
                            "VK_LAYER_KHRONOS_validation"};
                    }
                }
            }

            return std::vector<const char *> {};
        }();

        const std::vector<const char *> instanceExtensions = []
        {
            std::vector<const char *> temp {};

#ifdef __APPLE__
            temp.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            temp.push_back(
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
            );
#endif // __APPLE__

            if constexpr (ENABLE_VALIDATION_LAYERS)
            {
                temp.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                temp.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
            const char *const *instanceExtensionsArray    = nullptr;
            std::uint32_t      numberOfInstanceExtensions = 0;

            instanceExtensionsArray =
                glfwGetRequiredInstanceExtensions(&numberOfInstanceExtensions);

            if (instanceExtensionsArray != nullptr)
            {
                // TODO: what is the lifetime on these pointers?
                for (std::size_t i = 0; i < numberOfInstanceExtensions; ++i)
                {
                    temp.push_back(instanceExtensionsArray[i]);
#pragma clang diagnostic pop
                }
            }

            return temp;
        }();

        const vk::InstanceCreateInfo instanceCreateInfo {
            .sType {vk::StructureType::eInstanceCreateInfo},
            .pNext {[&]
                    {
                        if constexpr (ENABLE_VALIDATION_LAYERS)
                        {
                            return reinterpret_cast<const void *>(
                                &debugUtilsCreateInfo
                            );
                        }
                        else
                        {
                            return nullptr;
                        }
                    }()},
            .flags {
#ifdef __APPLE__
                    vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
#endif // __APPLE__
                    },
            .pApplicationInfo {&applicationInfo},
            .enabledLayerCount {
                    static_cast<std::uint32_t>(instanceLayers.size())},
            .ppEnabledLayerNames {instanceLayers.data()},
            .enabledExtensionCount {
                    static_cast<std::uint32_t>(instanceExtensions.size())},
            .ppEnabledExtensionNames {instanceExtensions.data()},
        };

        this->instance = vk::createInstanceUnique(instanceCreateInfo);

        dynamicLoaderInitalizationCallback(*this->instance);

        if constexpr (ENABLE_VALIDATION_LAYERS)
        {
            const VkDebugUtilsMessengerCreateInfoEXT CStyleCreateInfo =
                debugUtilsCreateInfo;

            const VkResult result = dynVkCreateDebugUtilsMessengerEXT(
                this->dyn_vk_get_instance_proc_addr,
                static_cast<VkInstance>(*this->instance),
                &CStyleCreateInfo,
                nullptr,
                &this->debug_messenger
            );

            util::assertFatal(
                result == VK_SUCCESS,
                "Failed to initialize debug Messenger | {}",
                vk::to_string(vk::Result {result})
            );
        }
    }

    Instance::~Instance()
    {
        dynVkDestroyDebugUtilsMessengerEXT(
            this->dyn_vk_get_instance_proc_addr,
            *this->instance,
            this->debug_messenger,
            nullptr
        );
    }

    vk::Instance Instance::operator* () const
    {
        return *this->instance;
    }
} // namespace gfx::vulkan
