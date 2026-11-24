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
    outBloom = vec4(0);
}
