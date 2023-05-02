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
    class DiskObject : public ObjectSystem
    {
    public:

        DiskObject(std::shared_ptr<gfx::Renderer>, const char* filepath);
        ~DiskObject() override;

        DiskObject(const DiskObject&)             = delete;
        DiskObject(DiskObject&&)                  = delete;
        DiskObject& operator= (const DiskObject&) = delete;
        DiskObject& operator= (DiskObject&&)      = delete;

        void tick(float) override;

        [[nodiscard]] std::vector<const gfx::Object*> lend() const override;

    }; // class DiskObject

} // namespace game::system

#endif // SRC_GAME_SYSTEM_DISK__OBJECT_HPP
