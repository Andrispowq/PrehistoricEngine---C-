#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

const int MaxSpheres = 8;

uniform sampler2D scene;
uniform sampler2D sceneDepth;
uniform samplerCube irradianceMap;
uniform vec3 cameraPosition;
uniform mat4 inverseViewProjection;
uniform vec3 sunRadiance;
uniform int sphereCount;
uniform vec4 sphereBounds[MaxSpheres];
uniform vec4 sphereMedia[MaxSpheres];
uniform float sphereAnisotropy[MaxSpheres];

#include "opengl/shadow/cascades.glsl"

float volumeAnisotropy(int i) { return sphereAnisotropy[i]; }
#include "shared/postProcessing/volumetric.glsl"

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outColour);
    if (any(greaterThanEqual(pixel, size))) return;

    vec2 ndc = (vec2(pixel) + 0.5) / vec2(size) * 2.0 - 1.0;
    float depth = texelFetch(sceneDepth, pixel, 0).r;
    vec3 origin = unprojectVolumeRay(ndc, -1.0);
    vec3 direction = normalize(origin - cameraPosition);
    float surfaceDistance = max(0.0, dot(unprojectVolumeRay(ndc, depth * 2.0 - 1.0) - origin, direction));
    vec3 background = texelFetch(scene, pixel, 0).rgb;
    float sampleOffset = volumeSampleOffset(pixel);
    imageStore(outColour, pixel, vec4(compositeVolumes(background, origin, direction, surfaceDistance, sampleOffset), 1.0));
}
