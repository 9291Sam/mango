#ifndef SRC_RENDER_VULKAN_INCLUDES_HPP
#define SRC_RENDER_VULKAN_INCLUDES_HPP

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>
#include <vulkan/vulkan_to_string.hpp>

// clang-format off
#include <vk_mem_alloc.h>
// clang-format on
#pragma clang diagnostic pop

#endif // SRC_RENDER_VULKAN_INCLUDES_HPP
