#ifndef SRC_GAME_WORLD_VOXEL__OCTREE_HPP
#define SRC_GAME_WORLD_VOXEL__OCTREE_HPP

#include <array>
#include <gfx/object.hpp>
#include <gfx/vulkan/includes.hpp>
#include <memory>
#include <variant>
#include <vector>

namespace gfx
{
    class Renderer;
    class Object;
} // namespace gfx

namespace game::world
{
    class VoxelOctree;
    class Node;

    class VoxelOctree
    {
    public:

        VoxelOctree(gfx::Renderer& renderer);
        ~VoxelOctree();

        VoxelOctree(const VoxelOctree&)             = delete;
        VoxelOctree(VoxelOctree&&)                  = default;
        VoxelOctree& operator= (const VoxelOctree&) = delete;
        VoxelOctree& operator= (VoxelOctree&&)      = default;

        std::vector<std::shared_ptr<gfx::Object>> draw(glm::vec3 lookAtVector);

    private:
        gfx::Renderer&                            renderer;
        std::vector<std::shared_ptr<gfx::Object>> objects;
        // TODO: you'll need a wrapper class to signal which parts
        // of the octree this is a part of
        std::unique_ptr<Node>                     root;
        glm::vec3                                 center_position;
        float                                     total_dimension;
        std::size_t                               number_of_voxels;
        const std::size_t                         voxels_per_object;
    };

} // namespace game::world

#endif // SRC_GAME_WORLD_VOXEL__OCTREE_HPP