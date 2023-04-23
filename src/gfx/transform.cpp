#include "transform.hpp"
#include "object.hpp"
#include <fmt/format.h>

namespace gfx
{
    glm::mat4 Transform::asModelMatrix() const
    {
        return this->asTranslationMatrix() * this->asRotationMatrix()
             * this->asScaleMatrix();
    }

    glm::mat4 Transform::asTranslationMatrix() const
    {
        return glm::translate(glm::mat4 {1.0f}, this->translation);
    }

    glm::mat4 Transform::asRotationMatrix() const
    {
        return glm::mat4_cast(this->rotation);
    }

    glm::mat4 Transform::asScaleMatrix() const
    {
        return glm::scale(glm::mat4 {1.0f}, this->scale);
    }

    glm::vec3 Transform::getForwardVector() const
    {
        return this->rotation * Transform::ForwardVector;
    }

    glm::vec3 Transform::getUpVector() const
    {
        return this->rotation * Transform::UpVector;
    }

    glm::vec3 Transform::getRightVector() const
    {
        return this->rotation * Transform::RightVector;
    }

    Transform::operator std::string () const
    {
        return fmt::format(
            "Transform:\nPosition: {}\nRotation : {}\nScale:{}\n",
            glm::to_string(this->translation),
            glm::to_string(this->rotation),
            glm::to_string(this->scale));
    }

    Camera::Camera(glm::vec3 position, float pitch_, float yaw_)
        : transform {}
        , pitch {pitch_}
        , yaw {yaw_}
    {
        this->transform.translation = position;
        this->updateRotationInTransform();
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
        this->pitch += pitchToAdd;
        this->updateRotationInTransform();
    }

    void Camera::addYaw(float yawToAdd)
    {
        this->yaw += yawToAdd;
        this->updateRotationInTransform();
    }

    void Camera::updateRotationInTransform()
    {
        glm::quat q {1.0f, 0.0f, 0.0f, 0.0f};

        this->pitch = std::clamp(
            this->pitch, -glm::half_pi<float>(), glm::half_pi<float>());
        this->yaw = glm::mod(this->yaw, glm::two_pi<float>());

        q *= glm::angleAxis(this->pitch, glm::vec3 {1.0f, 0.0f, 0.0f});

        q = glm::angleAxis(this->yaw, glm::vec3 {0.0f, -1.0f, 0.0f}) * q;

        this->transform.rotation = q;
    }
} // namespace gfx
