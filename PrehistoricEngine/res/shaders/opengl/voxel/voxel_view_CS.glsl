#version 450 core

layout(local_size_x = 8, local_size_y = 8) in;
layout(binding = 0, rgba32f) uniform writeonly image2D outputImage;
layout(binding = 1, rgba16f) uniform writeonly image2D indirectImage;
layout(binding = 0) uniform sampler3D radianceVolume;
layout(binding = 1) uniform sampler2D positionMetallic;
layout(binding = 3) uniform sampler2D normals;
layout(binding = 4) uniform usampler3D packedVoxels;

uniform bool debugView;
uniform vec3 volumeMin;
uniform float volumeExtent;
uniform mat4 inverseViewProjection;
uniform int traceStride;

layout(binding = 5) uniform samplerCube coneEnvironment;
const bool useEnvironment = true;
#include "opengl/voxel/voxel_cones.glsl"

const float voxelNearClip = -1.0;
int voxelResolution() { return textureSize(packedVoxels, 0).x; }
uint readVoxel(ivec3 cell) { return texelFetch(packedVoxels, cell, 0).r; }
#include "shared/voxel/view.glsl"

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = debugView ? imageSize(outputImage) : imageSize(indirectImage);
    if (any(greaterThanEqual(pixel, size))) return;
    if (debugView)
    {
        vec2 ndc = (vec2(pixel) + 0.5) / vec2(size) * 2.0 - 1.0;
        imageStore(outputImage, pixel, vec4(drawVoxels(ndc), 1.0));
        return;
    }

    ivec2 receiver = min(pixel * traceStride + traceStride / 2, textureSize(normals, 0) - 1);
    vec4 surface = texelFetch(normals, receiver, 0);
    if (surface.a < 0.99 || dot(surface.xyz, surface.xyz) < 0.01)
    {
        imageStore(indirectImage, pixel, vec4(0));
        return;
    }
    vec4 position = texelFetch(positionMetallic, receiver, 0);
    vec3 origin = (position.xyz - volumeMin) / volumeExtent;
    if (!inside(origin))
    {
        imageStore(indirectImage, pixel, vec4(0));
        return;
    }

    vec3 normal = normalize(surface.xyz);
    float cellSize = 1.0 / float(textureSize(radianceVolume, 0).x);
    vec3 radiance = traceHemisphere(origin, normal, cellSize);

    imageStore(indirectImage, pixel, vec4(radiance, 1.0));
}
