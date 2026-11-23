#version 450

layout(location = 0) in vec3 direction;

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outBloom;

layout(set = 0, binding = 0) uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
    outBloom = vec4(0);
    vec3 ray = normalize(direction);
    vec3 colour = textureLod(environmentMap, ray, 0.0).rgb;
    outColour = vec4(colour, 1.0);
}
