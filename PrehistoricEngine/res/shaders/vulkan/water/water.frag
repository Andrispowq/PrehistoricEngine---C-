#version 450
#extension GL_GOOGLE_include_directive : require
#include "../pbr/camera.glsl"
#include "params.glsl"
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
vec3 sampleReflection(vec2 uv) { return texture(waterReflection, vec2(1.0 - uv.x, uv.y)).rgb; }
float sampleRefractionDepth(vec2 uv) { return texture(refractionDepth, uv).a; }
#include "../../shared/water/surface.glsl"
void main()
{
    vec3 face = cross(dFdx(world.xyz), dFdy(world.xyz));
    vec3 normal;
    sceneColour = vec4(shadeWater(world, waveUV, face.y < 0 ? -face : face, normal), 1);
    bloom = vec4(0);
}
