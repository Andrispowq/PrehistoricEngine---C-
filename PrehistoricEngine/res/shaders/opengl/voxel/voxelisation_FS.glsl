#version 450 core

layout(binding = 0, r32ui) uniform uimage3D packedVoxels;
layout(binding = 0) uniform sampler2D albedoMap;
uniform vec3 volumeMin;
uniform float volumeExtent;
uniform vec3 albedo;
uniform bool hasAlbedoMap;
uniform float emission;
uniform bool injectRadiance;
uniform vec3 sunDirection;
uniform vec3 sunRadiance;

#include "opengl/shadow/cascades.glsl"

in Fragment
{
    vec3 position;
    vec3 normal;
    vec2 uv;
} fragment;

void main()
{
    vec3 uvw = (fragment.position - volumeMin) / volumeExtent;
    if (any(lessThan(uvw, vec3(0))) || any(greaterThanEqual(uvw, vec3(1)))) return;

    vec4 texel = hasAlbedoMap ? texture(albedoMap, fragment.uv) : vec4(1);
    if (texel.a < 0.5) discard;
    if (!injectRadiance)
    {
        uint surface = packUnorm4x8(vec4(clamp(albedo * texel.rgb, 0.0, 1.0), 1.0));
        imageAtomicMax(packedVoxels, ivec3(uvw * vec3(imageSize(packedVoxels))), surface);
        return;
    }

    vec3 base = albedo * pow(texel.rgb, vec3(2.2));
    vec3 normal = normalize(fragment.normal);
    float cosine = max(dot(normal, sunDirection), 0.0);
    float visibility = 0.0;
    if (cosine > 0.0 && any(greaterThan(sunRadiance, vec3(0))))
        visibility = directionalVisibility(fragment.position, normal, 1.5, true);

    // Keep one deterministic sample per cell, prioritising brighter radiance.
    vec3 radiance = base * (sunRadiance * (cosine * visibility / 3.14159) + emission);
    vec3 encoded = clamp(log2(1.0 + radiance) / 8.0, 0.0, 1.0);
    float priority = max(max(encoded.r, encoded.g), encoded.b);
    uint encodedVoxel = packUnorm4x8(vec4(encoded, (1.0 + 254.0 * priority) / 255.0));
    imageAtomicMax(packedVoxels, ivec3(uvw * vec3(imageSize(packedVoxels))), encodedVoxel);
}
