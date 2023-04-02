#include "includes.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#define VMA_IMPLEMENTATION           1
#define VMA_STATIC_VULKAN_FUNCTIONS  0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

#pragma clang diagnostic pop
