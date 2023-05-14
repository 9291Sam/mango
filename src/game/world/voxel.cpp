#include "voxel.hpp"
#include <map>
#include <numeric>
#include <random>
#include <ranges>
#include <util/log.hpp>
#include <util/misc.hpp>

/*
# cube.obj
# Import into Blender with Y-forward, Z-up
#
# Vertices:                        Faces:
#      f-------g                          +-------+
#     /.      /|                         /.  5   /|  3 back
#    / .     / |                        / .     / |
#   e-------h  |                   2   +-------+ 1|
#   |  b . .|. c      z          right |  . . .|. +
#   | .     | /       | /y             | . 4   | /
#   |.      |/        |/               |.      |/
#   a-------d         +---- x          +-------+
#                                           6
#                                        bottom

g cube

# Vertices
v 0.0 0.0 0.0  # 1 a
v 0.0 1.0 0.0  # 2 b
v 1.0 1.0 0.0  # 3 c
v 1.0 0.0 0.0  # 4 d
v 0.0 0.0 1.0  # 5 e
v 0.0 1.0 1.0  # 6 f
v 1.0 1.0 1.0  # 7 g
v 1.0 0.0 1.0  # 8 h

# Normal vectors
# One for each face. Shared by all vertices in that face.
vn  1.0  0.0  0.0  # 1 cghd
vn -1.0  0.0  0.0  # 2 aefb
vn  0.0  1.0  0.0  # 3 gcbf
vn  0.0 -1.0  0.0  # 4 dhea
vn  0.0  0.0  1.0  # 5 hgfe
vn  0.0  0.0 -1.0  # 6 cdab

# Faces v/vt/vn
#   3-------2
#   | -     |
#   |   #   |  Each face = 2 triangles (ccw)
#   |     - |            = 1-2-3 + 1-3-4
#   4-------1

# Face 1: cghd = cgh + chd
f 3//1 7//1 8//1
f 3//1 8//1 4//1

# Face 2: aefb = aef + afb
f 1//2 5//2 6//2
f 1//2 6//2 2//2

# Face 3: gcbf = gcb + gbf
f 7//3 3//3 2//3
f 7//3 2//3 6//3

# Face 4: dhea = dhe + dea
f 4//4 8//4 5//4
f 4//4 5//4 1//4

# Face 5: hgfe = hgf + hfe
f 8//5 7//5 6//5
f 8//5 6//5 5//5

# Face 6: cdab = cda + cab
f 3//6 4//6 1//6
f 3//6 1//6 2//6
*/

constexpr inline glm::vec3
getVoxelVertexPosition(float voxelEdgeLength, std::size_t cornerNumber)
{
    switch (cornerNumber)
    {
    case 0:
        return glm::vec3 {0.0f, 0.0f, 0.0f};
    case 1:
        return glm::vec3 {0.0f, voxelEdgeLength, 0.0f};
    case 2:
        return glm::vec3 {voxelEdgeLength, voxelEdgeLength, 0.0f};
    case 3:
        return glm::vec3 {voxelEdgeLength, 0.0f, 0.0f};
    case 4:
        return glm::vec3 {0.0f, 0.0f, voxelEdgeLength};
    case 5:
        return glm::vec3 {0.0f, voxelEdgeLength, voxelEdgeLength};
    case 6:
        return glm::vec3 {voxelEdgeLength, voxelEdgeLength, voxelEdgeLength};
    case 7:
        return glm::vec3 {voxelEdgeLength, 0.0f, voxelEdgeLength};
    default:
        util::panic("Invalid cornerNumber {}", cornerNumber);
    }

    std::unreachable();
}

constexpr inline glm::vec3 getVoxelNormal(std::size_t normalNumber)
{
    switch (normalNumber)
    {
    case 0:
        return glm::vec3 {1.0f, 0.0f, 0.0f};
    case 1:
        return glm::vec3 {-1.0f, 0.0f, 0.0f};
    case 2:
        return glm::vec3 {0.0f, 1.0f, 0.0f};
    case 3:
        return glm::vec3 {0.0f, -1.0f, 0.0f};
    case 4:
        return glm::vec3 {0.0f, 0.0f, 1.0f};
    case 5:
        return glm::vec3 {0.0f, 0.0f, -1.0f};
    default:
        util::panic("Invalid normalNumber {}", normalNumber);
    }

    std::unreachable();
}

constexpr inline void createVoxelTriangle(
    float                             size,
    std::array<std::size_t, 3>        trianglePositionNumbers,
    glm::vec3                         positionOffset,
    glm::vec4                         color,
    std::size_t                       normalNumber,
    std::vector<gfx::vulkan::Vertex>& output)
{
    // TODO: uvs?
    output.push_back(gfx::vulkan::Vertex {
        .position {
            positionOffset
            + getVoxelVertexPosition(size, trianglePositionNumbers[0])},
        .color {color},
        .normal {getVoxelNormal(normalNumber)},
        .uv {0.0, 0.0}});

    output.push_back(gfx::vulkan::Vertex {
        .position {
            positionOffset
            + getVoxelVertexPosition(size, trianglePositionNumbers[1])},
        .color {color},
        .normal {getVoxelNormal(normalNumber)},
        .uv {0.0, 0.0}});

    output.push_back(gfx::vulkan::Vertex {
        .position {
            positionOffset
            + getVoxelVertexPosition(size, trianglePositionNumbers[2])},
        .color {color},
        .normal {getVoxelNormal(normalNumber)},
        .uv {0.0, 0.0},
    });
}

namespace game::world
{
    VoxelCube::VoxelCube(
        glm::vec3            position_,
        std::size_t          sideDimension,
        std::optional<Voxel> fillVoxel)
        : position {position_}
        , side_dimension {sideDimension}
    {
        const std::size_t VoxelVolumeSize =
            this->side_dimension * this->side_dimension * this->side_dimension;

        if (fillVoxel.has_value())
        {
            this->voxels.resize(VoxelVolumeSize, *fillVoxel);
        }
        else
        {
            this->voxels.resize(VoxelVolumeSize);
        }

        const auto  SideIterator = std::views::iota(0UZ, this->side_dimension);
        const float colorSpreadPerInstance =
            255.0f / static_cast<float>(this->side_dimension);

        for (std::size_t x : SideIterator)
        {
            for (std::size_t y : SideIterator)
            {
                for (std::size_t z : SideIterator)
                {
                    this->at(x, y, z) = Voxel {
                        .r {static_cast<std::uint8_t>(
                            colorSpreadPerInstance * x)},
                        .g {static_cast<std::uint8_t>(
                            colorSpreadPerInstance * y)},
                        .b {static_cast<std::uint8_t>(
                            colorSpreadPerInstance * z)},
                        .a {255}};
                }
            }
        }
    }

    std::pair<std::vector<gfx::vulkan::Vertex>, std::vector<gfx::vulkan::Index>>
    VoxelCube::getVerticesAndIndices() const
    {
        const float VoxelSize =
            1.0f; // TODO: add this as a configurable parameter
        const auto  SideIterator = std::views::iota(0UZ, this->side_dimension);
        const float SpreadFactor = 1.0f;

        std::vector<gfx::vulkan::Vertex> duplicatedVertices;

        for (std::size_t x : SideIterator)
        {
            for (std::size_t y : SideIterator)
            {
                for (std::size_t z : SideIterator)
                {
                    const glm::vec3 OffsetVector {
                        static_cast<float>(x) * VoxelSize * SpreadFactor,
                        static_cast<float>(y) * VoxelSize * SpreadFactor,
                        static_cast<float>(z) * VoxelSize * SpreadFactor};

#define EMIT_VOXEL_TRIANGLE(t1, t2, t3, normalNumber)                          \
    createVoxelTriangle(                                                       \
        VoxelSize,                                                             \
        {t1, t2, t3},                                                          \
        this->position + OffsetVector,                                         \
        this->at(x, y, z).getFloatColors(),                                    \
        normalNumber,                                                          \
        duplicatedVertices);

                    if (!this->voxelExistsAt(x + 1, y, z)
                        || !this->voxelExistsAt(x - 1, y, z)
                        || !this->voxelExistsAt(x, y + 1, z)
                        || !this->voxelExistsAt(x, y - 1, z)
                        || !this->voxelExistsAt(x, y, z + 1)
                        || !this->voxelExistsAt(x, y, z - 1))
                    {
                        EMIT_VOXEL_TRIANGLE(2, 6, 7, 0)
                        EMIT_VOXEL_TRIANGLE(2, 7, 3, 0)

                        EMIT_VOXEL_TRIANGLE(0, 4, 5, 1)
                        EMIT_VOXEL_TRIANGLE(0, 5, 1, 1)

                        EMIT_VOXEL_TRIANGLE(6, 2, 1, 2)
                        EMIT_VOXEL_TRIANGLE(6, 1, 5, 2)

                        EMIT_VOXEL_TRIANGLE(3, 7, 4, 3)
                        EMIT_VOXEL_TRIANGLE(3, 4, 0, 3)

                        EMIT_VOXEL_TRIANGLE(7, 6, 5, 4)
                        EMIT_VOXEL_TRIANGLE(7, 5, 4, 4)

                        EMIT_VOXEL_TRIANGLE(2, 3, 0, 5)
                        EMIT_VOXEL_TRIANGLE(2, 0, 1, 5)
                    }

#undef EMIT_VOXEL_TRIANGLE
                }
            }
        }

        std::map<gfx::vulkan::Vertex, std::size_t> vertexToIndexOfIndexMap;

        std::vector<gfx::vulkan::Vertex> uniqueVertices;
        uniqueVertices.reserve(duplicatedVertices.size());

        std::vector<gfx::vulkan::Index> indices;
        indices.reserve(duplicatedVertices.size());

        for (const gfx::vulkan::Vertex& v : duplicatedVertices)
        {
            if (vertexToIndexOfIndexMap.contains(v))
            {
                indices.push_back(static_cast<gfx::vulkan::Index>(
                    vertexToIndexOfIndexMap[v]));
            }
            else
            {
                vertexToIndexOfIndexMap[v] = uniqueVertices.size();
                uniqueVertices.push_back(v);
                indices.push_back(static_cast<gfx::vulkan::Index>(
                    vertexToIndexOfIndexMap[v]));
            }
        }

        util::logLog(
            "Vertices {} | Indices: {}", uniqueVertices.size(), indices.size());

        return {uniqueVertices, indices};
    } // namespace game::world

    Voxel& VoxelCube::at(std::size_t x, std::size_t y, std::size_t z)
    {
        // clang-format off
        return this->voxels.at(
            x
            + this->side_dimension * y
            + this->side_dimension * this->side_dimension * z);
        // clang-format on
    }

    Voxel VoxelCube::at(std::size_t x, std::size_t y, std::size_t z) const
    {
        return const_cast<VoxelCube*>(this)->at(x, y, z);
    }

    bool
    VoxelCube::voxelExistsAt(std::size_t x, std::size_t y, std::size_t z) const
    {
        if (x > this->side_dimension - 1 || y > this->side_dimension - 1
            || z > this->side_dimension - 1)
        {
            return false;
        }

        if (this->at(x, y, z).a == 0)
        {
            return false;
        }

        return true;
    }
} // namespace game::world
