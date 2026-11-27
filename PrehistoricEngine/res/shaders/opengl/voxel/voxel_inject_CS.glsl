#version 450 core
layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout(binding = 0, r32ui) uniform readonly uimage3D geometryCells;
layout(binding = 1, r32ui) uniform readonly uimage3D surfaceCells;
layout(binding = 2, r32ui) uniform readonly uimage3D emissionCells;
layout(binding = 3, rgba16f) uniform writeonly image3D radianceCells;
layout(binding = 1) uniform sampler2D voxelShadowMap;
uniform vec3 sunDirection, sunRadiance;
uniform mat4 lightViewProjection;
uniform float shadowBias;
uniform bool useVoxelSunShadows, useTerrainOcclusion;
#include "shared/voxel/voxel_surface.glsl"
#include "opengl/voxel/voxel_visibility.glsl"
#include "shared/voxel/shadow.glsl"
uniform bool globalInjection;
layout(std430, binding = 6) readonly buffer VoxelCascadeData { uint cascadeWords[]; };
#include "shared/voxel/cascades.glsl"
#undef voxelVisibility

void main()
{
    ivec3 cell = ivec3(gl_GlobalInvocationID);
    int size = imageSize(geometryCells).x;
    if (any(greaterThanEqual(cell, ivec3(size)))) return;
    uint colour = imageLoad(geometryCells, cell).r;
    if (colour == 0u) { imageStore(radianceCells, cell, vec4(0)); return; }
    vec3 base = unpackUnorm4x8(colour).rgb;
    vec3 normal = unpackBounceNormal(imageLoad(surfaceCells, cell).r);
    vec3 position = voxelOcclusionMin + (vec3(cell) + 0.5) * voxelOcclusionExtent / float(size);
    float cosine = max(dot(normal, sunDirection), 0);
    float visibility = 0;
    if (cosine > 0 && any(greaterThan(sunRadiance, vec3(0))))
    visibility = useVoxelSunShadows ? (globalInjection ? vxCascadeVisibility(position, normal, sunDirection, 2.0 * vxCascadeBounds(vxCascadeCount() - 1).w)
    : voxelVisibility(position, normal, sunDirection, 2 * voxelOcclusionExtent))
    : voxelShadowVisibility((lightViewProjection * vec4(position, 1)).xyz * 0.5 + 0.5, shadowBias);
    if (useTerrainOcclusion && !useVoxelSunShadows && visibility > 0)
    visibility *= (globalInjection ? vxCascadeVisibility(position, normal, sunDirection, 2.0 * vxCascadeBounds(vxCascadeCount() - 1).w)
    : voxelVisibility(position, normal, sunDirection, 2 * voxelOcclusionExtent));
    vec3 radiance = decodeVoxelRadiance(imageLoad(emissionCells, cell).r) + base * sunRadiance * (cosine * visibility / 3.14159265);
    imageStore(radianceCells, cell, vec4(radiance, 1));
}
