#ifndef SRC_GAME_WORLD_WORLD_HPP
#define SRC_GAME_WORLD_WORLD_HPP

#include <memory>
#include <vector>

namespace gfx
{
    class Renderer;
    class Object;
} // namespace gfx

namespace game::world
{
    class World
    {
    public:
        explicit World(gfx::Renderer&);
        ~World() = default;

        World(const World&)             = delete;
        World(World&&)                  = delete;
        World& operator= (const World&) = delete;
        World& operator= (World&&)      = delete;

        // why the shared_ptr?
        // these can fall off between frames and need to stay alive just long
        // enough
        [[nodiscard]] std::vector<std::shared_ptr<gfx::Object>> draw() const;

    private:
        std::vector<std::shared_ptr<gfx::Object>> objects;
        gfx::Renderer&                            renderer;
    };
} // namespace game::world

#endif // SRC_GAME_WORLD_WORLD_HPP