#version 450

layout(location = 0) in vec3 direction;
layout(location = 0) out vec4 outColour;

layout(push_constant) uniform Atmosphere
{
    layout(offset = 48) vec4 sunPositionRadius;
    vec4 sunColourIntensity;
    vec4 rayleighMie;
    vec4 radiiAndScaleHeights;
    vec2 anisotropyAndShift;
} sky;

#include "../../shared/atmosphere/scattering.glsl"

void main()
{
    vec3 ray = normalize(direction + vec3(0.0, sky.anisotropyAndShift.y, 0.0));
    vec3 sun = normalize(sky.sunPositionRadius.xyz);
    vec4 scales = sky.radiiAndScaleHeights;
    vec3 radiance = atmosphere(ray, vec3(0.0, scales.x + 1000.0, 0.0), sun, sky.sunColourIntensity.w,
        scales.x, scales.y, sky.rayleighMie.rgb, sky.rayleighMie.w, scales.z, scales.w, sky.anisotropyAndShift.x);
    float disk = SunDiskWeight(length(ray - sun), sky.sunPositionRadius.w);
    outColour = vec4(mix(radiance, sky.sunColourIntensity.rgb * 4.0, disk), 1.0);
}
