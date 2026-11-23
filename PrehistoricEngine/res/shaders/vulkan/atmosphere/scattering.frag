#version 450

layout(location = 0) in vec3 direction;
layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outBloom;

#include "sky.glsl"

void main()
{
    outBloom = vec4(0);
    outColour = vec4(SkyRadiance(direction, false), 1.0);
}
