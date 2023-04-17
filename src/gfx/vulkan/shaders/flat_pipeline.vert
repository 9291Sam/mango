#version 460

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec2 in_uv;

// layout(push_constant) uniform PushConstants
// {
//     mat4 model_view_proj;
// }
// in_push_constants;

layout(location = 0) out vec3 out_color;
// layout(location = 1) out vec3 out_normal;
// layout(location = 2) out vec2 out_uv;

// void main()
// {
//     gl_Position = in_push_constants.model_view_proj * vec4(in_position, 1.0);
//     out_color   = in_color;
//     out_normal  = in_normal;
//     out_uv      = in_uv;
// }

vec2 positions[3] = vec2[](vec2(0.0, -0.5), vec2(0.5, 0.5), vec2(-0.5, 0.5));

vec3 colors[3] =
    vec3[](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

// layout(location = 0) out vec3 out_color;

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    out_color   = colors[gl_VertexIndex];
}