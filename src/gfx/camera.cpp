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
        this->pitch += pitchToAdd;
        this->updateTransformFromRotations();
        // const float pitchBefore =
        // glm::eulerAngles(this->transform.rotation).p; if (pitchBefore +
        // pitchToAdd > glm::half_pi<float>())
        // {
        //     this->transform.pitchBy(glm::half_pi<float>() - pitchBefore);

        //     util::logTrace("Yawing by {:.8f}", yawToAdd);
        // }
        // else
        // {
        //     this->transform.pitchBy(pitchToAdd);
        // }
    }

    void Camera::addYaw(float yawToAdd)
    {
        this->yaw += yawToAdd;
        this->updateTransformFromRotations();

        // util::logTrace("Yawing by {:.8f}", yawToAdd);

        // util::logTrace(
        //     "yawing rotation! {}", glm::to_string(this->transform.rotation));

        // this->transform.yawBy(yawToAdd);
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

        // TODO: what why are these flipped
        if (window.isActionActive(Window::Action::PlayerMoveUp))
        {
            this->addPosition(-this->getUpVector() * deltaTime * MoveScale);
        }

        if (window.isActionActive(Window::Action::PlayerMoveDown))
        {
            this->addPosition(this->getUpVector() * deltaTime * MoveScale);
        }

        auto [xDelta, yDelta] = window.getMouseDelta();

        this->addYaw(xDelta * deltaTime * rotateSpeedScale);
        this->addPitch(yDelta * deltaTime * rotateSpeedScale);
    }

    Camera::operator std::string () const
    {
        return fmt::format(
            "{} | Pitch {} | Yaw {}",
            static_cast<std::string>(this->transform),
            this->pitch,
            this->yaw);
    }

    void Camera::updateTransformFromRotations()
    {
        glm::quat q {1.0f, 0.0f, 0.0f, 0.0f};

        this->pitch = std::clamp(
            this->pitch, -glm::half_pi<float>(), glm::half_pi<float>());
        this->yaw = glm::mod(this->yaw, glm::two_pi<float>());

        q *= glm::angleAxis(this->pitch, -Transform::RightVector);

        q = glm::angleAxis(this->yaw, Transform::UpVector) * q;

        this->transform.rotation = q;
    }
} // namespace gfx
