#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;

layout(binding = 3) uniform sampler2D positions;
uniform vec4 viewDepthRow;
uniform vec2 lens;
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

vec3 sampleLensColour(vec2 uv)
{
    vec3 colour = sampleSceneColour(uv);
    if (underwater != 0) colour = underwaterColour(uv, vec2(textureSize(scene, 0)), colour);
    return colour;
}

float inverseViewDepth(vec2 uv)
{
    ivec2 p = ivec2(uv * resolution);
    if (texelFetch(normals, p, 0).a < 0.5) return 0.0;
    return 1.0 / max(0.001, dot(viewDepthRow, vec4(texelFetch(positions, p, 0).xyz, 1.0)));
}
#include "shared/postProcessing/depth_of_field.glsl"


void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(x, ivec2(resolution)))) return;
    vec2 texCoord = (vec2(x) + 0.5) / resolution;

    vec3 sceneColour = sampleLensColour(texCoord);

    if (lens.y > 0.0) sceneColour = depthOfFieldColour(texCoord, resolution, flareFootprint, lens.x, lens.y, sceneColour);
    if (underwater == 0) sceneColour += sunLensFlare(texCoord, flare, flareFootprint);
    sceneColour = ToneMapHDR(sceneColour, exposure, gamma);

    if (outline) sceneColour = selectionOutline(scene, x, sceneColour);
    imageStore(outColour, x, vec4(sceneColour, 1.0));
}