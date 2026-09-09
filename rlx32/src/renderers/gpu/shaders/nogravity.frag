#version 450

layout(location = 0) in vec3 v_uvq;
layout(location = 1) in vec4 v_color;

layout(location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D tex;

void main()
{
    out_color = texture(tex, v_uvq.xy / v_uvq.z) * v_color;
}
