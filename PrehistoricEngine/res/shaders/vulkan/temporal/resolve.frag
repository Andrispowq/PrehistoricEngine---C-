#version 450
#extension GL_GOOGLE_include_directive : require
layout(set = 0, binding = 6, std140) uniform TemporalParameters
{
    mat4 reprojection;
    vec4 jitter;
    vec4 settings;
    vec4 options;
    vec4 depthRow;
} params;
layout(set = 0, binding = 0) uniform sampler2D rawMotion;
layout(set = 0, binding = 5) uniform sampler2D sceneDepth;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 colour;
int sampleCount() { return 1; }
vec4 motionSample(ivec2 p, int i) { return texelFetch(rawMotion, p, 0); }
float visibleDepth(ivec2 p) { float depth = 1;
    for (int i = 0;i < sampleCount();++i) depth = min(depth, texelFetch(sceneDepth, p, 0).r);
    if (depth >= 1) return 0;
    return params.depthRow.y / (2 * depth - 1 + params.depthRow.x); }
#include "../../shared/temporal/resolve.glsl"
void main() { colour = resolveMotion(uv); }
