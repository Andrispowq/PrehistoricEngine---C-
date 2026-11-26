#version 450

layout(location = 0) in vec2 uv;
layout(binding = 0) uniform sampler2D scene;
layout(binding = 1) uniform sampler2D bloom;
layout(location = 0) out vec4 outColour;
layout(push_constant) uniform Settings
{
    float exposure;
    float gamma;
    float bloomEnabled;
    float underwater;
    float outline;
    float sunX, sunY, flareStrength, flareSize, footprintX, footprintY;
} hdr;

layout(binding = 2) uniform sampler2D flareAtlas;
layout(binding = 3) uniform sampler2D emission;
float skyCoverage(vec2 uv) { return clamp(textureLod(emission, uv, 0.0).a, 0.0, 1.0); }
#include "../../shared/postProcessing/sun_flare.glsl"
#include "../../shared/postProcessing/hdr.glsl"

vec3 sampleSceneColour(vec2 uv)
{
    vec3 colour = texture(scene, uv).rgb;
    if (hdr.bloomEnabled > 0.0) colour += texture(bloom, uv).rgb;
    return colour;
}

#include "../../shared/water/underwater.glsl"

void main()
{
    vec3 colour = sampleSceneColour(uv);
    if (hdr.underwater > 0) colour = underwaterColour(uv, vec2(textureSize(scene, 0)), colour);
    if (hdr.underwater == 0.0)
    colour += sunLensFlare(uv, vec4(hdr.sunX, hdr.sunY, hdr.flareStrength, hdr.flareSize), vec2(hdr.footprintX, hdr.footprintY));
    colour = ToneMapHDR(colour, hdr.exposure, hdr.gamma);
    if (hdr.outline > 0.0) colour = selectionOutline(scene, ivec2(gl_FragCoord.xy), colour);
    outColour = vec4(colour, 1.0);
}
