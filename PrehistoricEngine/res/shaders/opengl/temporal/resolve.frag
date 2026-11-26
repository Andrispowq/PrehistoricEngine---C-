#version 430
layout(binding = 6, std140) uniform TemporalParameters
{
    mat4 reprojection;
    vec4 jitter;
    vec4 settings;
    vec4 options;
    vec4 depthRow;
} params;
layout(binding = 0) uniform sampler2D rawMotion;
layout(binding = 5) uniform sampler2D positions;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 colour;
int sampleCount() { return 1; }
vec4 motionSample(ivec2 p, int i) { return texelFetch(rawMotion, p, 0); }
float visibleDepth(ivec2 p) { vec4 world = texelFetch(positions, p, 0);
    if (dot(world.xyz, world.xyz) == 0) return 0;
    return dot(vec4(world.xyz, 1), params.depthRow); }
#include "shared/temporal/resolve.glsl"
void main() { colour = resolveMotion(uv); }
