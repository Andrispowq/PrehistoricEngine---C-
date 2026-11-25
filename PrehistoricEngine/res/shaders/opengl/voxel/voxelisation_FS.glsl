#version 450 core

layout(binding = 0, r32ui) uniform uimage3D packedVoxels;
layout(binding = 0) uniform sampler2D albedoMap;
uniform vec3 volumeMin;
uniform float volumeExtent;
uniform vec3 albedo;
uniform bool hasAlbedoMap;
uniform float emission;
uniform bool injectRadiance;
uniform bool captureBounceSurfaces;
uniform float metallic;
layout(binding = 2) uniform sampler2D metallicMap;
layout(binding = 1, r32ui) uniform uimage3D bounceSurfaces;

#include "opengl/voxel/voxel_surface.glsl"
#include "opengl/voxel/voxel_visibility.glsl"
uniform bool useVoxelSunShadows;
uniform vec3 sunDirection;
uniform vec3 sunRadiance;

layout(binding = 1) uniform sampler2D voxelShadowMap;
uniform mat4 lightViewProjection;
uniform float shadowBias;

float voxelShadowVisibility(vec3 worldPosition)
{
    vec3 shadowPosition = (lightViewProjection * vec4(worldPosition, 1.0)).xyz * 0.5 + 0.5;
    ivec2 size = textureSize(voxelShadowMap, 0);
    vec2 texel = shadowPosition.xy * vec2(size) - 0.5;
    ivec2 base = ivec2(floor(texel));
    vec2 fraction = fract(texel);
    float visibility = 0.0;
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 2; ++x)
        {
            ivec2 pixel = clamp(base + ivec2(x, y), ivec2(0), size - 1);
            vec2 weight = mix(1.0 - fraction, fraction, vec2(x, y));
            float depth = texelFetch(voxelShadowMap, pixel, 0).r;
            visibility += weight.x * weight.y * step(shadowPosition.z - shadowBias, depth);
        }
    return visibility;
}

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
    vec3 normal = dot(fragment.normal, fragment.normal) > 0.0 ? normalize(fragment.normal) : vec3(0, 1, 0);
    ivec3 cell = ivec3(uvw * vec3(imageSize(packedVoxels)));
    if (captureBounceSurfaces)
    {
        float metalness = metallic < 0.0 ? texture(metallicMap, fragment.uv).r : metallic;
        imageAtomicMax(bounceSurfaces, cell, packBounceSurface(base * (1.0 - clamp(metalness, 0.0, 1.0)), normal));
    }
    float cosine = max(dot(normal, sunDirection), 0.0);
    float visibility = 0.0;
    if (cosine > 0.0 && any(greaterThan(sunRadiance, vec3(0))))
        visibility = useVoxelSunShadows
            ? voxelVisibility(fragment.position, normal, sunDirection, 2.0 * voxelOcclusionExtent)
            : voxelShadowVisibility(fragment.position);

    // Keep one deterministic sample per cell, prioritising brighter radiance.
    vec3 radiance = base * (sunRadiance * (cosine * visibility / 3.14159) + emission);
    vec3 encoded = clamp(log2(1.0 + radiance) / 8.0, 0.0, 1.0);
    float priority = max(max(encoded.r, encoded.g), encoded.b);
    uint encodedVoxel = packUnorm4x8(vec4(encoded, (1.0 + 254.0 * priority) / 255.0));
    imageAtomicMax(packedVoxels, cell, encodedVoxel);
}
