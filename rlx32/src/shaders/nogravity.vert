// SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later
#version 450

// Vertices arrive already projected: x,y in logical pixels (origin top
// left), z is the depth in [0,1].  uvq carries u/w, v/w, 1/w so that the
// fragment shader can do perspective correct texturing on screen space
// triangles; for 2D quads q is 1.

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_uvq;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec3 v_uvq;
layout(location = 1) out vec4 v_color;

layout(set = 1, binding = 0) uniform Transform
{
    vec4 xform; // 2/width, 2/height, unused, unused
};

void main()
{
    gl_Position = vec4(in_pos.x * xform.x - 1.0, 1.0 - in_pos.y * xform.y, in_pos.z, 1.0);
    gl_PointSize = 1.0;
    v_uvq = in_uvq;
    v_color = in_color;
}
