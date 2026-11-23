#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput scene;
layout(location = 0) out vec4 outColour;
layout(push_constant) uniform Settings
{
    float exposure;
    float gamma;
} hdr;

#include "../../shared/postProcessing/hdr.glsl"

void main()
{
    outColour = vec4(ToneMapHDR(subpassLoad(scene).rgb, hdr.exposure, hdr.gamma), 1.0);
}
