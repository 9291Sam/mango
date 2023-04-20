#ifndef SRC_RENDER_VULKAN_INCLUDES_HPP
#define SRC_RENDER_VULKAN_INCLUDES_HPP

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
// clang-format off

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>
#include <vulkan/vulkan_to_string.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "GLFW/glfw3.h"

#include <vk_mem_alloc.h>

// clang-format on
#pragma clang diagnostic pop

#endif // SRC_RENDER_VULKAN_INCLUDES_HPP
