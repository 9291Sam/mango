#ifndef SRC_GAME_ENTITY_DISK__OBJECT_HPP
#define SRC_GAME_ENTITY_DISK__OBJECT_HPP

#include "entity.hpp"

namespace gfx
{
    class Renderer;
    class Object;
} // namespace gfx

namespace game::entity
{
    class DiskObject final : public Entity
    {
    public:

        DiskObject(std::shared_ptr<gfx::Renderer>, const char* filepath);
        ~DiskObject() override;

        void tick(float) override;

        [[nodiscard]] std::vector<const gfx::Object*> lend() const override;

    private:
        gfx::TriangulatedObject object;
    }; // class DiskObject

} // namespace game::entity

#endif // SRC_GAME_ENTITY_DISK__OBJECT_HPP
