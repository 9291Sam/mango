#ifndef SRC_GAME_SYSTEM_HPP
#define SRC_GAME_SYSTEM_HPP

#include <gfx/object.hpp>
#include <memory>
#include <span>
#include <vector>

namespace gfx
{
    class Renderer;
} // namespace gfx

namespace game::system
{

    class System
    {
    public:
        virtual ~System();

        virtual void tick(float deltaTime) = 0;
    };

    class ObjectSystem : public System
    {
    public:

        ObjectSystem(std::shared_ptr<gfx::Renderer>);
        virtual ~ObjectSystem() override;

        [[nodiscard]] virtual std::vector<const gfx::Object*> lend() const = 0;

    protected:
        std::shared_ptr<gfx::Renderer> renderer;
        std::vector<gfx::Object>       objects;
    };

} // namespace game::system

#endif // SRC_GAME_SYSTEM_SYSTEM_HPP
