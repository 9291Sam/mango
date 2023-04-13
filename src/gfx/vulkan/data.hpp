#ifndef SRC_GFX_VULKAN_DATA_HPP
#define SRC_GFX_VULKAN_DATA_HPP

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
} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_DATA_HPP