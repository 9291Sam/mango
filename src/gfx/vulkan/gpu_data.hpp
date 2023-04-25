#ifndef SRC_GFX_VULKAN_DATA_HPP
#define SRC_GFX_VULKAN_DATA_HPP

#include "includes.hpp"
#include <string>

namespace gfx::vulkan
{
    using Index = std::uint32_t;
    /// NOTE:
    /// If you change any of these, dont forget to update their
    /// corresponding structs in the shaders!
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv;

        static const vk::VertexInputBindingDescription* getBindingDescription()
        {
            static const vk::VertexInputBindingDescription bindings {
                .binding {0},
                .stride {sizeof(Vertex)},
                .inputRate {vk::VertexInputRate::eVertex},
            };

            return &bindings;
        }

        [[nodiscard]] static auto getAttributeDescriptions()
            -> const std::array<vk::VertexInputAttributeDescription, 4>*
        {
            // clang-format off
            static const std::array<vk::VertexInputAttributeDescription, 4>
            descriptions
            {
                vk::VertexInputAttributeDescription
                {
                    .location {0},
                    .binding {0},
                    .format {vk::Format::eR32G32B32Sfloat},
                    .offset {offsetof(Vertex, position)},
                },
                vk::VertexInputAttributeDescription
                {
                    .location {1},
                    .binding {0},
                    .format {vk::Format::eR32G32B32Sfloat},
                    .offset {offsetof(Vertex, color)},
                },
                vk::VertexInputAttributeDescription
                {
                    .location {2},
                    .binding {0},
                    .format {vk::Format::eR32G32B32Sfloat},
                    .offset {offsetof(Vertex, normal)},
                },
                vk::VertexInputAttributeDescription
                {
                    .location {3},
                    .binding {0},
                    .format {vk::Format::eR32G32Sfloat},
                    .offset {offsetof(Vertex, uv)},
                },
            };
            // clang-format on
            return &descriptions;
        }

        [[nodiscard]] operator std::string () const;
        [[nodiscard]] bool operator== (const Vertex&) const = default;
    };

    struct PushConstants
    {
        glm::mat4 model_view_proj;
    };
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_DATA_HPP
