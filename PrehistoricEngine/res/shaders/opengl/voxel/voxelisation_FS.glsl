#version 450 core
layout(binding = 0, r32ui) uniform uimage3D packedVoxels;
layout(binding = 1, r32ui) uniform uimage3D bounceSurfaces;
layout(binding = 2, r32ui) uniform uimage3D emissionVoxels;
layout(binding = 0) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D metallicMap;
uniform vec3 volumeMin, albedo, scrollDelta;
uniform float volumeExtent, emission, metallic;
uniform bool hasAlbedoMap, incremental;
#include "shared/voxel/voxel_surface.glsl"
#include "shared/voxel/scroll.glsl"
in Fragment { vec3 position; vec3 normal; vec2 uv; } fragment;
void main()
{
    vec3 uvw = (fragment.position - volumeMin) / volumeExtent;
    if (any(lessThan(uvw, vec3(0))) || any(greaterThanEqual(uvw, vec3(1)))) return;
    ivec3 cell = ivec3(uvw * vec3(imageSize(packedVoxels)));
    if (incremental && retainedVoxel(cell, ivec3(scrollDelta), imageSize(packedVoxels).x)) return;
    vec4 texel = hasAlbedoMap ? texture(albedoMap, fragment.uv) : vec4(1);
    if (texel.a < 0.5) discard;
    vec3 base = albedo * pow(texel.rgb, vec3(2.2));
    imageAtomicMax(packedVoxels, cell, packUnorm4x8(vec4(clamp(base, 0.0, 1.0), 1.0)));
    vec3 normal = dot(fragment.normal, fragment.normal) > 0 ? normalize(fragment.normal) : vec3(0, 1, 0);
    float metal = metallic < 0 ? texture(metallicMap, fragment.uv).r : metallic;
    imageAtomicMax(bounceSurfaces, cell, packBounceSurface(base * (1 - clamp(metal, 0, 1)), normal));
    vec3 encoded = encodeVoxelRadiance(base * emission);
    float priority = max(encoded.r, max(encoded.g, encoded.b));
    imageAtomicMax(emissionVoxels, cell, packUnorm4x8(vec4(encoded, (1 + 254 * priority) / 255)));
}
