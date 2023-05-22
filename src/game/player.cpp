#include "player.hpp"
#include <gfx/renderer.hpp>

namespace game
{
    Player::Player(gfx::Renderer& renderer_, glm::vec3 position)
        : renderer {renderer_}
        , camera {position}
    {}

    void Player::tick()
    {
        // TODO: moving diaginally is faster
        const float MoveScale =
            renderer.getActionAmount(gfx::Window::Action::PlayerSprint) ? 100.0f
                                                                        : 50.0f;

        const float rotateSpeedScale = 150.255f;

        this->camera.addPosition(
            this->camera.getForwardVector()
            * renderer.getActionAmount(gfx::Window::Action::PlayerMoveForward)
            * MoveScale);

        this->camera.addPosition(
            -this->camera.getForwardVector()
            * renderer.getActionAmount(gfx::Window::Action::PlayerMoveBackward)
            * MoveScale);

        this->camera.addPosition(
            -this->camera.getRightVector()
            * renderer.getActionAmount(gfx::Window::Action::PlayerMoveLeft)
            * MoveScale);

        this->camera.addPosition(
            this->camera.getRightVector()
            * renderer.getActionAmount(gfx::Window::Action::PlayerMoveRight)
            * MoveScale);

        // TODO: what why are these flipped
        this->camera.addPosition(
            -gfx::Transform::UpVector
            * renderer.getActionAmount(gfx::Window::Action::PlayerMoveUp)
            * MoveScale);

        this->camera.addPosition(
            gfx::Transform::UpVector
            * renderer.getActionAmount(gfx::Window::Action::PlayerMoveDown)
            * MoveScale);

        auto [xDelta, yDelta] = renderer.getMouseDelta();

        this->camera.addYaw(xDelta * rotateSpeedScale);
        this->camera.addPitch(yDelta * rotateSpeedScale);
    }

    gfx::Camera& Player::getCamera()
    {
        return this->camera;
    }

} // namespace game
