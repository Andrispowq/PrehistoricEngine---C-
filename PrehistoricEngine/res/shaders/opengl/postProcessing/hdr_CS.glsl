#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;

uniform vec4 flare;
uniform vec2 flareFootprint;
layout(binding = 1) uniform sampler2D flareAtlas;
layout(binding = 2) uniform sampler2D normals;
float skyCoverage(vec2 uv) { return 1.0 - clamp(textureLod(normals, uv, 0.0).a, 0.0, 1.0); }
#include "shared/postProcessing/sun_flare.glsl"

uniform int underwater;
uniform bool outline;
uniform float gamma;
uniform float exposure;
uniform vec2 resolution;

#include "shared/postProcessing/hdr.glsl"

vec3 sampleSceneColour(vec2 uv)
{
    return texture(scene, uv).rgb;
}

#include "shared/water/underwater.glsl"

void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(x, ivec2(resolution)))) return;
    vec2 texCoord = (vec2(x) + 0.5) / resolution;

    vec3 sceneColour = sampleSceneColour(texCoord);

    if (underwater != 0) sceneColour = underwaterColour(texCoord, resolution, sceneColour);
    if (underwater == 0) sceneColour += sunLensFlare(texCoord, flare, flareFootprint);
    sceneColour = ToneMapHDR(sceneColour, exposure, gamma);

    if (outline) sceneColour = selectionOutline(scene, x, sceneColour);
    imageStore(outColour, x, vec4(sceneColour, 1.0));
}