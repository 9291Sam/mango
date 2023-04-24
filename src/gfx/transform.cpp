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

} // namespace gfx
