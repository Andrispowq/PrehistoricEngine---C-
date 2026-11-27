uniform int cascadeCount;
uniform float cascadePlaneDistances[16];
uniform float shadowNearPlane;
uniform float cascadeBlendFraction;
uniform float shadowSunAngularRadius;
uniform mat4 shadowView;
uniform sampler2DArray shadowMap;

layout(std140, binding = 0) uniform LightSpaceMatrices
{
    mat4 m_LightSpace[16];
};

float cascadeSplit(int layer) { return cascadePlaneDistances[layer]; }

#include "shared/shadow/cascades.glsl"
#include "opengl/voxel/voxel_visibility.glsl"
layout(std430, binding = 6) readonly buffer VoxelCascadeData { uint cascadeWords[]; };
#include "shared/voxel/cascades.glsl"
uniform bool useVoxelSunShadows;
uniform vec3 sunDirection;

float directionalVisibility(vec3 position, vec3 normal, float biasInTexels, bool volume)
{
    if (useVoxelSunShadows)
    return voxelVisibility(position, normal, sunDirection, 2.0 * vxCascadeBounds(vxCascadeCount() - 1).w);

    return cascadeSunVisibility(position, normal, biasInTexels, volume);
}
