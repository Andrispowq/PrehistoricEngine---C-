#version 450 core
layout(local_size_x = 8, local_size_y = 8) in;
layout(binding = 0, r32ui) uniform uimage3D packedVoxels;
layout(binding = 1, r32ui) uniform uimage3D bounceSurfaces;
layout(binding = 0) uniform sampler2D terrainTextures[12];
#include "shared/voxel/terrain_parameters.glsl"
#include "shared/terrain/detail.glsl"
#include "shared/voxel/voxel_surface.glsl"
// Terrain textures occupy units 0..11; visibility has its own unit.
layout(binding = 12) uniform usampler3D terrainOccluders;
int voxelGridSize() { return textureSize(terrainOccluders, 0).x; }
int voxelTopLevel() { return textureQueryLevels(terrainOccluders) - 1; }
bool occupiedBlock(ivec3 p, int level) { return texelFetch(terrainOccluders, p, level).r != 0u; }
#define voxelOcclusionMin terrainGrid.xyz
#define voxelOcclusionExtent terrainGrid.w
#include "shared/voxel/voxel_visibility.glsl"
uniform vec3 sunDirection, sunRadiance;
uniform bool captureBounceSurfaces;
vec2 terrainHeightUV(vec2 uv) { return uv; }
vec2 terrainLayerUV(vec2 uv) { return uv; }
#include "shared/voxel/terrain_material.glsl"
void storeTerrainVoxel(ivec3 cell, vec3 normal, vec3 base, float metal, vec3 emission)
{
    if (terrainOptions.w == 0)
    {
        imageAtomicMax(packedVoxels, cell, packUnorm4x8(vec4(base, 1)));
        return;
    }
    if (captureBounceSurfaces)
    imageAtomicMax(bounceSurfaces, cell, packBounceSurface(base * (1 - clamp(metal, 0, 1)), normal));
    vec3 position = terrainGrid.xyz + (vec3(cell) + 0.5) * terrainGrid.w / terrainOptions.z;
    float cosine = max(dot(normal, sunDirection), 0);
    float visibility = cosine > 0 ? voxelVisibility(position, normal, sunDirection, 2 * terrainGrid.w) : 0;
    vec3 encoded = encodeVoxelRadiance(emission + base * sunRadiance * (cosine * visibility / 3.14159265));
    float priority = max(encoded.r, max(encoded.g, encoded.b));
    imageAtomicMax(packedVoxels, cell, packUnorm4x8(vec4(encoded, (1 + 254 * priority) / 255)));
}
#include "shared/voxel/terrain.glsl"
