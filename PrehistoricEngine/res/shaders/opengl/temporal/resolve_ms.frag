#version 430
layout(binding = 6, std140) uniform TemporalParameters
{
    mat4 reprojection;
    vec4 jitter;
    vec4 settings;
    vec4 options;
    vec4 depthRow;
} params;
layout(binding = 0) uniform sampler2DMS rawMotion;
layout(binding = 5) uniform sampler2D positions;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 colour;
uniform int rasterSamples;
int sampleCount() { return rasterSamples; }
vec4 motionSample(ivec2 p, int i) { return texelFetch(rawMotion, p, i); }
float visibleDepth(ivec2 p) { vec4 world = texelFetch(positions, p, 0);
    if (dot(world.xyz, world.xyz) == 0) return 0;
    return dot(vec4(world.xyz, 1), params.depthRow); }
#include "shared/temporal/resolve.glsl"
void main() { colour = resolveMotion(uv); }
