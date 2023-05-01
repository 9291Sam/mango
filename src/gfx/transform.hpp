#ifndef SRC_GFX_TRANSFORM_HPP
#define SRC_GFX_TRANSFORM_HPP

#include "vulkan/includes.hpp"

namespace gfx
{
    struct Transform
    {
        static constexpr glm::vec3 ForwardVector {0.0f, 0.0f, -1.0f};
        static constexpr glm::vec3 RightVector {1.0f, 0.0f, 0.0f};
        static constexpr glm::vec3 UpVector {0.0f, -1.0f, 0.0f};

        glm::vec3 translation;
        glm::quat rotation;
        glm::vec3 scale;

        [[nodiscard]] glm::mat4 asModelMatrix() const;
        [[nodiscard]] glm::mat4 asTranslationMatrix() const;
        [[nodiscard]] glm::mat4 asRotationMatrix() const;
        [[nodiscard]] glm::mat4 asScaleMatrix() const;

        [[nodiscard]] glm::vec3 getForwardVector() const;
        [[nodiscard]] glm::vec3 getUpVector() const;
        [[nodiscard]] glm::vec3 getRightVector() const;

        // add rotate by functions
        void pitchBy(float pitch);
        void yawBy(float yaw);
        void rollBy(float roll);

        void assertReasonable() const; // lmfao

        [[nodiscard]] explicit operator std::string () const;
    };

} // namespace gfx

#endif // SRC_GFX_TRANSFORM_HPP
