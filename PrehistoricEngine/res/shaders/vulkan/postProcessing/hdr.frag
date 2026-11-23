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
} hdr;

#include "../../shared/postProcessing/hdr.glsl"

void main()
{
    vec3 colour = texture(scene, uv).rgb;
    if (hdr.bloomEnabled > 0.0) colour += texture(bloom, uv).rgb;
    outColour = vec4(ToneMapHDR(colour, hdr.exposure, hdr.gamma), 1.0);
}
