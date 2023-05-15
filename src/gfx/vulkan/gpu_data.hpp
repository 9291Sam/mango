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
        glm::vec4 color;
        glm::vec3 normal;
        glm::vec2 uv;

        [[nodiscard]] static const vk::VertexInputBindingDescription*
        getBindingDescription()
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
                    .format {vk::Format::eR32G32B32A32Sfloat},
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
        [[nodiscard]] std::partial_ordering
        operator<=> (const Vertex& other) const
        {
#define EMIT_ORDERING(field)                                                   \
    if (this->field <=> other.field != std::partial_ordering::equivalent)      \
    {                                                                          \
        return this->field <=> other.field;                                    \
    }
            EMIT_ORDERING(position.x)
            EMIT_ORDERING(position.y)
            EMIT_ORDERING(position.z)

            EMIT_ORDERING(color.r)
            EMIT_ORDERING(color.g)
            EMIT_ORDERING(color.b)

            EMIT_ORDERING(normal.x)
            EMIT_ORDERING(normal.y)
            EMIT_ORDERING(normal.z)

            EMIT_ORDERING(uv.x)
            EMIT_ORDERING(uv.y)

#undef EMIT_ORDERING

            return std::partial_ordering::equivalent;
        }
    };

    struct PushConstants
    {
        glm::mat4 model_view_proj;
    };
} // namespace gfx::vulkan

// Vertex hash implementation
namespace std
{
    template<>
    struct hash<gfx::vulkan::Vertex>
    {
        [[nodiscard]] auto
        operator() (const gfx::vulkan::Vertex& vertex) const noexcept -> size_t
        {
            std::size_t      seed {0};
            std::hash<float> hasher;

            auto hashCombine = [](std::size_t& seed_, std::size_t hash_)
            {
                hash_ += 0x9e3779b9 + (seed_ << 6) + (seed_ >> 2);
                seed_ ^= hash_;
            };

            hashCombine(seed, hasher(vertex.position.x));
            hashCombine(seed, hasher(vertex.position.y));
            hashCombine(seed, hasher(vertex.position.z));

            hashCombine(seed, hasher(vertex.color.x));
            hashCombine(seed, hasher(vertex.color.y));
            hashCombine(seed, hasher(vertex.color.z));

            hashCombine(seed, hasher(vertex.normal.x));
            hashCombine(seed, hasher(vertex.normal.y));
            hashCombine(seed, hasher(vertex.normal.z));

            hashCombine(seed, hasher(vertex.uv.x));
            hashCombine(seed, hasher(vertex.uv.y));

            return seed;
        }
    };

} // namespace std

#endif // SRC_GFX_VULKAN_DATA_HPP
