#ifndef SRC_GAME_SYSTEM_CUBIC__VOXEL__CHUNK_HPP
#define SRC_GAME_SYSTEM_CUBIC__VOXEL__CHUNK_HPP

#include "system.hpp"

namespace gfx
{
    class Renderer;
    class Object;
} // namespace gfx

namespace game::system
{
    class CubicVoxelChunk final : public ObjectSystem
    {
    public:

        CubicVoxelChunk(std::shared_ptr<gfx::Renderer>, std::size_t edgeLength);
        ~CubicVoxelChunk() override;

        void tick(float) override;

        [[nodiscard]] std::vector<const gfx::Object*> lend() const override;

    private:
        // TODO: internal representations of the voxel tree
    }; // class DiskObject

} // namespace game::system

#endif // SRC_GAME_SYSTEM_CUBIC__VOXEL__CHUNK_HPP
