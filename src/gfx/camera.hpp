#ifndef SRC_GFX_CAMERA_HPP
#define SRC_GFX_CAMERA_HPP

#include "transform.hpp"
#include "vulkan/includes.hpp"

namespace gfx
{

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

#endif // SRC_GfX_CAMERA_HPP
