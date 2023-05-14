#version 460

layout(set = 0, binding = 0) uniform VoxelPositions
{
    vec4 positions[];
}
in_voxel_positions;

layout(set = 0, binding = 1) uniform VoxelMagnitudes
{
    float magnitudes[];
}
in_voxel_magnitudes;

const vec3 CUBE_STRIP_OFFSETS[] = {
    vec3(-0.5f, 0.5f, 0.5f),   // Front-top-left
    vec3(0.5f, 0.5f, 0.5f),    // Front-top-right
    vec3(-0.5f, -0.5f, 0.5f),  // Front-bottom-left
    vec3(0.5f, -0.5f, 0.5f),   // Front-bottom-right
    vec3(0.5f, -0.5f, -0.5f),  // Back-bottom-right
    vec3(0.5f, 0.5f, 0.5f),    // Front-top-right
    vec3(0.5f, 0.5f, -0.5f),   // Back-top-right
    vec3(-0.5f, 0.5f, 0.5f),   // Front-top-left
    vec3(-0.5f, 0.5f, -0.5f),  // Back-top-left
    vec3(-0.5f, -0.5f, 0.5f),  // Front-bottom-left
    vec3(-0.5f, -0.5f, -0.5f), // Back-bottom-left
    vec3(0.5f, -0.5f, -0.5f),  // Back-bottom-right
    vec3(-0.5f, 0.5f, -0.5f),  // Back-top-left
    vec3(0.5f, 0.5f, -0.5f),   // Back-top-right
};

void main()
{
    const vec4 world_location =
        in_voxel_positions.positions[gl_VertexIndex / 14];

    const float voxel_magnitude =
        in_voxel_magnitudes.magnitudes[gl_VertexIndex / 14];

    const vec4 cube_vertex =
        vec4(CUBE_STRIP_OFFSETS[gl_VertexIndex % 14], 0.0f);

    gl_Position = world_location + voxel_magnitude * cube_vertex;
}