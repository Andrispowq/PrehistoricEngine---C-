#version 450

layout(location = 0) in vec3 direction;

layout(location = 0) out vec4 outColour;

layout(push_constant) uniform Background
{
    mat4 inverseViewProjection;
    float exposure;
    float gamma;
} sky;

layout(set = 0, binding = 0) uniform sampler2D environmentMap;

const float PI = 3.14159265359;

void main()
{
    vec3 ray = normalize(direction);
    // Source HDR rows run from the north pole to the south pole.
    vec2 uv = vec2(atan(ray.z, ray.x) / (2.0 * PI) + 0.5,
            0.5 - asin(clamp(ray.y, -1.0, 1.0)) / PI);

    vec3 colour = max(textureLod(environmentMap, uv, 0.0).rgb, vec3(0.0));
    colour = 1.0 - exp(-colour * sky.exposure);
    if (sky.gamma != 1.0)
    {
        colour = pow(colour, vec3(1.0 / sky.gamma));
    }

    outColour = vec4(colour, 1.0);
}
