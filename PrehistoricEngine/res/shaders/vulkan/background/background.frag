#version 450

layout(location = 0) in vec3 direction;

layout(location = 0) out vec4 outColour;

layout(push_constant) uniform Background
{
    mat4 inverseViewProjection;
    float exposure;
    float gamma;
} sky;

layout(set = 0, binding = 0) uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
    vec3 ray = normalize(direction);
    vec3 colour = textureLod(environmentMap, ray, 0.0).rgb;
    colour = 1.0 - exp(-colour * sky.exposure);
    if (sky.gamma != 1.0)
    {
        colour = pow(colour, vec3(1.0 / sky.gamma));
    }

    outColour = vec4(colour, 1.0);
}
