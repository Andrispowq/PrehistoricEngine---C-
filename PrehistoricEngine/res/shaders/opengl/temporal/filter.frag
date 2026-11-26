#version 430
layout(binding = 6, std140) uniform TemporalParameters
{
    mat4 reprojection;
    vec4 jitter;
    vec4 settings;
    vec4 options;
    vec4 depthRow;
} params;
layout(binding = 0) uniform sampler2D sourceImage;
layout(binding = 1) uniform sampler2D motionImage;
layout(binding = 2) uniform sampler2D historyImage;
layout(binding = 3) uniform sampler2D previousMotion;
layout(binding = 4) uniform sampler2D tileImage;
uniform int mode;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 colour;
#include "shared/temporal/filter.glsl"
void main()
{
    if (mode == 0 || mode == 4) colour = temporalAA(uv, mode == 4);
    else if (mode == 1) colour = motionBlur(uv);
    else if (mode == 2) colour = tileMaximum(ivec2(gl_FragCoord.xy));
    else colour = neighborMaximum(ivec2(gl_FragCoord.xy));
}
