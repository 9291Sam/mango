#include "camera.hpp"
#include "window.hpp"

namespace gfx
{
    Camera::Camera(glm::vec3 position)
        : transform {
            .translation {position},
            .rotation {1.0f, 0.0f, 0.0f, 0.0f},
            .scale {1.0f, 1.0f, 1.0f}}
    {}

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
        util::logTrace("Yawing by {:.8f}", yawToAdd);

        util::logTrace(
            "yawing rotation! {}", glm::to_string(this->transform.rotation));

        this->transform.yawBy(yawToAdd);
    }

    void Camera::updateState(const Window& window)
    {
        // TODO: moving diaginally is faster
        const float MoveScale        = 50.0;
        const float rotateSpeedScale = 150.255f;
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

        // TODO: clamp!
        this->addYaw(xDelta * deltaTime * rotateSpeedScale);
        this->addPitch(yDelta * deltaTime * rotateSpeedScale);
    }
} // namespace gfx
