#include "camera.hpp"

namespace gfx
{
    Camera::Camera(glm::vec3 position)
        : transform {}
    {
        this->transform.translation = position;
    }

    glm::mat4 Camera::getPerspectiveMatrix(
        float fovY,
        float aspectRatio,
        float closeClippingPlaneDistance,
        float farClippingPlaneDistance)
    {
        glm::mat4 projection = glm::perspective(
            fovY,
            aspectRatio,
            closeClippingPlaneDistance,
            farClippingPlaneDistance);
        // glm is designed for openGL, vulkan's y coordinate is flipped in
        // comparison
        projection[1][1] *= -1;

        return projection;
    }

    glm::mat4 Camera::getViewMatrix() const
    {
        return glm::inverse(
            this->transform.asTranslationMatrix()
            * this->transform.asRotationMatrix());
    }

    [[nodiscard]] glm::vec3 Camera::getForwardVector() const
    {
        return this->transform.getForwardVector();
    }

    [[nodiscard]] glm::vec3 Camera::getRightVector() const
    {
        return this->transform.getRightVector();
    }

    [[nodiscard]] glm::vec3 Camera::getUpVector() const
    {
        return this->transform.getUpVector();
    }

    void Camera::addPosition(glm::vec3 positionToAdd)
    {
        this->transform.translation += positionToAdd;
    }

    void Camera::addPitch(float pitchToAdd)
    {
        const float pitchBefore = glm::eulerAngles(this->transform.rotation).p;
        if (pitchBefore + pitchToAdd > glm::half_pi<float>())
        {
            this->transform.pitchBy(glm::half_pi<float>() - pitchBefore);
        }
        else
        {
            this->transform.pitchBy(pitchToAdd);
        }
    }

    void Camera::addYaw(float yawToAdd)
    {
        this->transform.yawBy(yawToAdd);
    }

    // void updateRotationInTransform()
    // {
    //     glm::quat q {1.0f, 0.0f, 0.0f, 0.0f};

    //     this->pitch = std::clamp(
    //         this->pitch, -glm::half_pi<float>(), glm::half_pi<float>());
    //     this->yaw = glm::mod(this->yaw, glm::two_pi<float>());

    //     q *= glm::angleAxis(this->pitch, glm::vec3 {1.0f, 0.0f, 0.0f});

    //     q = glm::angleAxis(this->yaw, glm::vec3 {0.0f, -1.0f, 0.0f}) * q;

    //     this->transform.rotation = q;
    // }
} // namespace gfx
