#ifndef SRC_GFX_CAMERA_HPP
#define SRC_GFX_CAMERA_HPP

#include "transform.hpp"
#include "vulkan/includes.hpp"

namespace gfx
{
    class Window;

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

        [[nodiscard]] glm::vec3 getForwardVector() const;
        [[nodiscard]] glm::vec3 getRightVector() const;
        [[nodiscard]] glm::vec3 getUpVector() const;

        void addPosition(glm::vec3);
        void addPitch(float);
        void addYaw(float);

        void updateState(const Window&);

        explicit operator std::string () const;

    private:
        void updateTransformFromRotations();

        float pitch;
        float yaw;

        Transform transform;
    };
} // namespace gfx

#endif // SRC_GfX_CAMERA_HPP
