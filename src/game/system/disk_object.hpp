#ifndef SRC_GAME_SYSTEM_DISK__OBJECT_HPP
#define SRC_GAME_SYSTEM_DISK__OBJECT_HPP

#include "system.hpp"

namespace gfx
{
    class Renderer;
    class Object;
} // namespace gfx

namespace game::system
{
    class DiskObject final : public System
    {
    public:

        DiskObject(std::shared_ptr<gfx::Renderer>, const char* filepath);
        ~DiskObject() override;

        void tick(float) override;

        [[nodiscard]] std::vector<const gfx::Object*> lend() const override;

    private:
        gfx::Object object;
    }; // class DiskObject

} // namespace game::system

#endif // SRC_GAME_SYSTEM_DISK__OBJECT_HPP
