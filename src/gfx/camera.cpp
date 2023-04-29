#include "camera.hpp"
#include "window.hpp"

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

    void Camera::updateState(const Window& window)
    {
        // TODO: moving diaginally is faster
        const float MoveScale        = 650.0;
        const float rotateSpeedScale = 1500000.255;
        const float deltaTime        = window.getDeltaTimeSeconds();

        if (window.isActionActive(Window::Action::PlayerMoveForward))
        {
            this->addPosition(this->getForwardVector() * deltaTime * MoveScale);
        }

        if (window.isActionActive(Window::Action::PlayerMoveBackward))
        {
            this->addPosition(
                -this->getForwardVector() * deltaTime * MoveScale);
        }

        if (window.isActionActive(Window::Action::PlayerMoveLeft))
        {
            this->addPosition(-this->getRightVector() * deltaTime * MoveScale);
        }

        if (window.isActionActive(Window::Action::PlayerMoveRight))
        {
            this->addPosition(this->getRightVector() * deltaTime * MoveScale);
        }

        auto [xDelta, yDelta] = window.getMouseDelta();

        // util::logTrace("mouse deltas {} | {}", xDelta, yDelta);

        // TODO: clamp!
        this->addYaw(xDelta * deltaTime * rotateSpeedScale);
        this->addPitch(yDelta * deltaTime * rotateSpeedScale);
    }
} // namespace gfx
