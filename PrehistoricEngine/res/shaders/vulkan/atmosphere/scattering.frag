#version 450

layout(location = 0) in vec3 direction;
layout(location = 0) out vec4 outColour;

#include "sky.glsl"

void main()
{
    outColour = vec4(SkyRadiance(direction, false), 1.0);
}
