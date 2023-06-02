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
    struct Voxel
    {
        glm::vec4 linear_color;

        bool shouldDraw()
        {
            return this->linear_color.a != 0.0f;
        }
    };

    class VoxelOctree;
    class Node;

    struct LocalPosition
    {
        std::int32_t x;
        std::int32_t y;
        std::int32_t z;

        LocalPosition operator- (LocalPosition other)
        {
            return LocalPosition {
                .x {this->x - other.x},
                .y {this->y - other.y},
                .z {this->z - other.z},
            };
        }

        explicit operator std::string ();
    };

    class VoxelOctree
    {
    public:

        VoxelOctree(
            gfx::Renderer&, glm::vec3 centerPosition, std::size_t levels);
        ~VoxelOctree();

        VoxelOctree(const VoxelOctree&)             = delete;
        VoxelOctree(VoxelOctree&&)                  = delete;
        VoxelOctree& operator= (const VoxelOctree&) = delete;
        VoxelOctree& operator= (VoxelOctree&&)      = delete;

        // TODO: accept a position in the world for floating origin stuff
        std::vector<std::shared_ptr<gfx::Object>> draw(glm::vec3 lookAtVector);

        void insertVoxelAtPosition(Voxel, LocalPosition);

        const std::size_t levels;
        const std::size_t dimension;

    private:

        void collapseTree();

        gfx::Renderer&                            renderer;
        std::vector<std::shared_ptr<gfx::Object>> objects;
        // TODO: you'll need a wrapper class to signal which parts
        // of the octree this is a part of

        std::unique_ptr<Node> root;
        glm::vec3             center_position;
        // std::size_t           number_of_voxels;
        // const std::size_t     voxels_per_object;
    };

} // namespace game::world

#endif // SRC_GAME_WORLD_VOXEL__OCTREE_HPP
