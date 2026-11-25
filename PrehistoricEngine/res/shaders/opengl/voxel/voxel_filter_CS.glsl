#version 450 core

layout(local_size_x = 8, local_size_y = 8) in;
layout(binding = 0, rgba32f) uniform image2D outputImage;
layout(binding = 0) uniform sampler2D indirectImage;
layout(binding = 1) uniform sampler2D positionMetallic;
layout(binding = 2) uniform sampler2D albedoRoughness;
layout(binding = 3) uniform sampler2D normals;

uniform float giStrength;
uniform float voxelSize;
uniform int traceStride;
uniform vec3 volumeMin;
uniform float volumeExtent;

#include "shared/voxel/filter.glsl"

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);
    if (any(greaterThanEqual(pixel, size))) return;

    vec4 receiver = texelFetch(normals, pixel, 0);
    if (receiver.a < 0.99 || dot(receiver.xyz, receiver.xyz) < 0.01) return;
    vec4 position = texelFetch(positionMetallic, pixel, 0);
    vec3 irradiance = filteredIndirect(pixel, size, position.xyz, normalize(receiver.xyz), traceStride, voxelSize, volumeMin, volumeExtent);
    vec3 albedo = texelFetch(albedoRoughness, pixel, 0).rgb;
    vec4 scene = imageLoad(outputImage, pixel);
    scene.rgb += irradiance * albedo * (1.0 - clamp(position.w, 0.0, 1.0)) * giStrength;
    imageStore(outputImage, pixel, scene);
}
