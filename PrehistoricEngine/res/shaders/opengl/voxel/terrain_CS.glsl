#version 450 core
layout(local_size_x = 8, local_size_y = 8) in;
layout(binding = 0, r32ui) uniform uimage3D packedVoxels;
layout(binding = 1, r32ui) uniform uimage3D bounceSurfaces;
layout(binding = 2, r32ui) uniform uimage3D emissionVoxels;
layout(binding = 0) uniform sampler2D terrainTextures[12];
#include "shared/voxel/terrain_parameters.glsl"
#include "shared/terrain/detail.glsl"
#include "shared/voxel/voxel_surface.glsl"
vec2 terrainHeightUV(vec2 uv) { return uv; }
vec2 terrainLayerUV(vec2 uv) { return uv; }
#include "shared/voxel/terrain_material.glsl"
void storeTerrainVoxel(ivec3 cell, vec3 normal, vec3 base, float metal, vec3 emission)
{
    imageAtomicMax(packedVoxels, cell, packUnorm4x8(vec4(base, 1)));
    imageAtomicMax(bounceSurfaces, cell, packBounceSurface(base * (1 - clamp(metal, 0, 1)), normal));
    vec3 encoded = encodeVoxelRadiance(emission);
    float priority = max(encoded.r, max(encoded.g, encoded.b));
    imageAtomicMax(emissionVoxels, cell, packUnorm4x8(vec4(encoded, (1 + 254 * priority) / 255)));
}
#include "shared/voxel/terrain.glsl"
