#ifndef SRC_GAME_ENTITY_DISK__ENTITY_HPP
#define SRC_GAME_ENTITY_DISK__ENTITY_HPP

#include "entity.hpp"
#include <memory>

namespace gfx
{
    class Renderer;
    class Object;
    class TriangulatedObject;
} // namespace gfx

namespace game::entity
{
    class DiskEntity final : public Entity
    {
    public:

        DiskEntity(gfx::Renderer&, const char* filepath);
        ~DiskEntity() override;

        void                            tick() override;
        std::vector<const gfx::Object*> draw() const override;

    public:
        std::unique_ptr<gfx::TriangulatedObject> object;
    }; // class DiskEntity

} // namespace game::entity

#endif // SRC_GAME_ENTITY_DISK__ENTITY_HPP
