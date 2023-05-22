#ifndef SRC_GFX_CAMERA_HPP
#define SRC_GFX_CAMERA_HPP

#include "transform.hpp"
#include "vulkan/includes.hpp"

namespace gfx
{
    class Renderer;

    class Camera
    {
    public:

        Camera(glm::vec3 position);

        static glm::mat4 getPerspectiveMatrix(
            float fovY,
            float aspectRatio,
            float closeClippingPlaneDistance,
            float farClippingPlaneDistance);
        glm::mat4 getViewMatrix() const;

        glm::vec3 getForwardVector() const;
        glm::vec3 getRightVector() const;
        glm::vec3 getUpVector() const;

        void addPosition(glm::vec3);
        void addPitch(float);
        void addYaw(float);

        explicit operator std::string () const;

    private:
        void updateTransformFromRotations();

        float pitch;
        float yaw;

        Transform transform;
    };
} // namespace gfx

#endif // SRC_GfX_CAMERA_HPP
