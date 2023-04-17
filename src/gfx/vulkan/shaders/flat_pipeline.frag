#version 460

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

// layout(push_constant) uniform PushConstants
// {
//     mat4 model_view_proj;
// }
// in_push_constants;

layout(location = 0) out vec4 out_color;

void main()
{
    out_color = vec4(in_color, 1.0);
}