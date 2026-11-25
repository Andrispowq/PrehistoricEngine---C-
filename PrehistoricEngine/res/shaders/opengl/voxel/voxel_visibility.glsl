layout(binding = 8) uniform usampler3D voxelOccluders;
uniform vec3 voxelOcclusionMin;
uniform float voxelOcclusionExtent;

int voxelGridSize() { return textureSize(voxelOccluders, 0).x; }
int voxelTopLevel() { return textureQueryLevels(voxelOccluders) - 1; }
bool occupiedBlock(ivec3 p, int level) { return texelFetch(voxelOccluders, p, level).r != 0u; }
#include "shared/voxel/voxel_visibility.glsl"
