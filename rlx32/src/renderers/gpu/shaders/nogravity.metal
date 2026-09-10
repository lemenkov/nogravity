// SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later
#include <metal_stdlib>
using namespace metal;

// Metal twin of nogravity.vert / nogravity.frag.

struct VertexIn
{
    float3 pos   [[attribute(0)]];
    float3 uvq   [[attribute(1)]];
    float4 color [[attribute(2)]];
};

struct VertexOut
{
    float4 pos [[position]];
    float3 uvq;
    float4 color;
    float point_size [[point_size]];
};

struct Transform
{
    float4 xform;
};

vertex VertexOut vs_main(VertexIn in [[stage_in]], constant Transform &t [[buffer(0)]])
{
    VertexOut out;
    out.pos = float4(in.pos.x * t.xform.x - 1.0, 1.0 - in.pos.y * t.xform.y, in.pos.z, 1.0);
    out.uvq = in.uvq;
    out.color = in.color;
    out.point_size = 1.0;
    return out;
}

fragment float4 fs_main(VertexOut in [[stage_in]], texture2d<float> tex [[texture(0)]], sampler smp [[sampler(0)]])
{
    return tex.sample(smp, in.uvq.xy / in.uvq.z) * in.color;
}
