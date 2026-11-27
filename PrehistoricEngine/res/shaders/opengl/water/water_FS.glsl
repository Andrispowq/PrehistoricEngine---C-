#version 430
#define M_PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outPositionMetallic;
layout(location = 2) out vec4 outAlbedoRoughness;
layout(location = 3) out vec4 outNormal;
layout(location = 4) out vec4 outBloom;

in vec4 position_FS;
in vec2 mapCoord_FS;
in vec3 normal_FS;

uniform sampler2D waterReflection;
uniform sampler2D waterRefraction;
uniform sampler2D refractionDepth;
uniform float refractionDepthScale;
uniform float waterHeight;
uniform sampler2D normalMap;
uniform sampler2D dudvMap;
uniform mat4 viewProjection;
uniform vec3 cameraPosition;
uniform vec3 waterColour;
uniform vec3 sunDirection;
uniform vec3 sunRadiance;
uniform float specularFactor;
uniform float kReflection;
uniform float kRefraction;
uniform float reflectionBlendFactor;
uniform float capillarStrength;
uniform float capillarDownsampling;
uniform float dudvDownsampling;
uniform int isCameraUnderWater;

layout(binding = 0) uniform usampler3D waterVoxelOccluders;
layout(binding = 1) uniform sampler3D waterVoxelRadiance;
uniform vec4 waterVoxelBounds, waterVoxelOptions;
int voxelGridSize() { return int(waterVoxelOptions.x); }
int voxelTopLevel() { return textureQueryLevels(waterVoxelOccluders) - 1; }
bool occupiedBlock(ivec3 p, int level) { return texelFetch(waterVoxelOccluders, p, level).r != 0u; }
#define voxelOcclusionMin waterVoxelBounds.xyz
#define voxelOcclusionExtent waterVoxelBounds.w
#include "shared/voxel/voxel_visibility.glsl"
vec4 sampleConeRadiance(vec3 p, vec3 ray, float lod) { return textureLod(waterVoxelRadiance, p, lod); }
vec4 baseSample(ivec3 p) { return texelFetch(waterVoxelRadiance, p, 0); }
vec3 baseRadiance(ivec3 p) { return baseSample(p).rgb; }
#include "shared/voxel/cone_hit.glsl"
#include "shared/voxel/reflection.glsl"
layout(std430, binding = 6) readonly buffer VoxelCascadeData { uint cascadeWords[]; };
#include "shared/voxel/cascades.glsl"
#define WATER_VOXEL_REFLECTION
#include "shared/water/voxel_reflection.glsl"
vec3 sampleReflection(vec2 uv) { return texture(waterReflection, uv).rgb; }
float sampleRefractionDepth(vec2 uv) { return texture(refractionDepth, uv).r; }
#include "shared/water/surface.glsl"

void main()
{
    vec3 n;
    vec3 colour = shadeWater(position_FS, mapCoord_FS, normal_FS, n);
    outColour = vec4(colour, 1);
    outPositionMetallic = vec4(position_FS.xyz, 0);
    outAlbedoRoughness = vec4(colour, 0.08);
    outNormal = vec4(n, 1);
    outBloom = vec4(0, 0, 0, -1); // Already receives voxel specular in shadeWater.
}
