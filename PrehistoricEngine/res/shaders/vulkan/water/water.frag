#version 450
#extension GL_GOOGLE_include_directive : require
#include "../pbr/camera.glsl"
#include "params.glsl"
#include "../../shared/water/wireframe.glsl"
layout(location=0) in vec4 world;
layout(location=1) in vec2 waveUV;
layout(location=0) out vec4 sceneColour;
layout(location=1) out vec4 bloom;
layout(set=2,binding=6) uniform sampler2D waterReflection;
layout(set=2,binding=7) uniform sampler2D waterRefraction;
layout(set=2,binding=8) uniform sampler2D refractionDepth;
layout(set=2,binding=9) uniform sampler2D normalMap;
layout(set=1,binding=0) uniform sampler2D dudvMap;
#define viewProjection (m_projection * m_view)
#define kReflection optics.x
#define kRefraction optics.y
#define reflectionBlendFactor optics.z
#define waterHeight optics.w
#define capillarStrength waterDetail.x
#define capillarDownsampling waterDetail.y
#define dudvDownsampling waterDetail.z
#define refractionDepthScale waterDetail.w
#define waterColour tint.rgb
#define isCameraUnderWater int(tint.w)
#define sunDirection sunlight.xyz
#define specularFactor sunlight.w
#define sunRadiance radiance.rgb
layout(std430, set = 0, binding = 1) readonly buffer WaterVoxelGrid { uint cells[]; };
layout(std140, set = 0, binding = 2) uniform WaterVoxelSettings { vec4 waterVoxelBounds; vec4 waterVoxelOptions; };
int voxelGridSize() { return int(waterVoxelOptions.x); }
#define voxelOcclusionMin waterVoxelBounds.xyz
#define voxelOcclusionExtent waterVoxelBounds.w
const bool directionalMips = false;
#include "../voxel/grid.glsl"
#include "../../shared/voxel/reflection.glsl"
#define WATER_VOXEL_REFLECTION
#include "../../shared/water/voxel_reflection.glsl"
vec3 sampleReflection(vec2 uv) { return texture(waterReflection, vec2(1.0 - uv.x, uv.y)).rgb; }
float sampleRefractionDepth(vec2 uv)
{
    ivec2 size = textureSize(refractionDepth, 0);
    return texelFetch(refractionDepth, clamp(ivec2(uv * size), ivec2(0), size - 1), 0).a;
}
#include "../../shared/water/surface.glsl"
void main()
{
    bloom = vec4(0);
    if (tessellation.w != 0.0)
    {
        sceneColour = waterWireframeColour;
        return;
    }
    vec3 face = cross(dFdx(world.xyz), dFdy(world.xyz));
    vec3 normal;
    sceneColour = vec4(shadeWater(world, waveUV, face.y < 0 ? -face : face, normal), 1);
}
