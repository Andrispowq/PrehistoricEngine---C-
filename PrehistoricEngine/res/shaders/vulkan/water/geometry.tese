#version 450
#extension GL_GOOGLE_include_directive : require
#include "evaluate.glsl"
layout(location=0) out vec4 world;
layout(location=1) out vec2 waveUV;
void main()
{
    evaluateWater(world, waveUV);
    gl_Position = world;
}
