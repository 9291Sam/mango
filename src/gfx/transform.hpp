#ifndef SRC_GFX_TRANSFORM_HPP
#define SRC_GFX_TRANSFORM_HPP

#include "vulkan/includes.hpp"

namespace gfx
{
    // TODO: seperate these files
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

        [[nodiscard]] explicit operator std::string () const;
    };

    class Camera
    {
    public:

        Camera(glm::vec3 position, float pitch, float yaw);

        static glm::mat4 getPerspectiveMatrix(
            float fovY,
            float aspectRatio,
            float closeClippingPlaneDistance,
            float farClippingPlaneDistance);
        glm::mat4 getViewMatrix() const;

        [[nodiscard]] glm::vec3 getForwardVector() const;
        [[nodiscard]] glm::vec3 getRightVector() const;
        [[nodiscard]] glm::vec3 getUpVector() const;

        void addPosition(glm::vec3);
        void addPitch(float);
        void addYaw(float);

    private:
        void updateRotationInTransform();

        Transform transform;
        float     pitch;
        float     yaw;
    };

} // namespace gfx

#endif // SRC_GFX_TRANSFORM_HPP
