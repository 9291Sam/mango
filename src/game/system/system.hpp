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
    // TODO: add id system
    class System
    {
    public:
        System(std::shared_ptr<gfx::Renderer>);
        virtual ~System();

        System(const System&)             = delete;
        System(System&&)                  = delete;
        System& operator= (const System&) = delete;
        System& operator= (System&&)      = delete;

        std::size_t getUUID() const;

        // TODO: add a std::span<const SystemEvent> to this
        // An example event would be a block breaking at a specific position
        // so each system, every tick iterates over this and sees, ok am I ready
        // also this may be trivial to thread
        virtual void tick(float deltaTime) = 0;
        // TODO: maybe make ticks two parts (threaded part and single threaded
        // part)

        [[nodiscard]] virtual std::vector<const gfx::Object*> lend() const = 0;

    protected:
        std::shared_ptr<gfx::Renderer> renderer;
        const std::size_t              id;
    };

} // namespace game::system

#endif // SRC_GAME_SYSTEM_SYSTEM_HPP
