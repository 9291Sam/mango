#ifndef SRC_GFX_VULKAN_DATA_HPP
#define SRC_GFX_VULKAN_DATA_HPP

#include "includes.hpp"
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

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

#pragma clang diagnostic pop

namespace gfx::vulkan
{
    /// NOTE:
    /// If you change any of these, dont forget to update their
    /// corresponding structs in the shaders!
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv;

        [[nodiscard]] static auto getBindingDescription()
            -> const vk::VertexInputBindingDescription*;

        [[nodiscard]] static auto getAttributeDescriptions()
            -> const std::array<vk::VertexInputAttributeDescription, 4>*;

        [[nodiscard]] operator std::string () const;
        [[nodiscard]] bool operator== (const Vertex&) const = default;
    };

    struct PushConstants
    {
        glm::mat4 model_view_proj;
    };
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_DATA_HPP