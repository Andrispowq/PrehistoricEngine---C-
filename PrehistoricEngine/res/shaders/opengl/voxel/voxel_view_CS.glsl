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

#include "opengl/voxel/voxel_cones.glsl"

vec3 drawVoxels(vec2 ndc)
{
    vec4 nearPoint = inverseViewProjection * vec4(ndc, -1, 1);
    vec4 farPoint = inverseViewProjection * vec4(ndc, 1, 1);
    vec3 origin = (nearPoint.xyz / nearPoint.w - volumeMin) / volumeExtent;
    vec3 direction = normalize(farPoint.xyz / farPoint.w - nearPoint.xyz / nearPoint.w);
    // Keep the slab and grid intersection divisions finite for axis-aligned rays.
    direction = mix(direction, vec3(1e-7), lessThan(abs(direction), vec3(1e-7)));
    vec3 t0 = -origin / direction;
    vec3 t1 = (1.0 - origin) / direction;
    vec3 enter = min(t0, t1);
    vec3 exit = max(t0, t1);
    float start = max(0.0, max(enter.x, max(enter.y, enter.z)));
    float end = min(exit.x, min(exit.y, exit.z));
    vec3 background = vec3(0.025, 0.035, 0.05);
    if (start >= end) return background;

    int size = textureSize(packedVoxels, 0).x;
    vec3 p = (origin + direction * start) * float(size);
    ivec3 cell = clamp(ivec3(floor(p)), ivec3(0), ivec3(size - 1));
    ivec3 stepDirection = ivec3(sign(direction));
    vec3 delta = abs(1.0 / (direction * float(size)));
    vec3 next = (vec3(cell) + max(vec3(stepDirection), vec3(0)) - p)
              / (direction * float(size));
    int axis = enter.x >= max(enter.y, enter.z) ? 0 : (enter.y >= enter.z ? 1 : 2);
    for (int i = 0; i < 3 * size; ++i)
    {
        if (any(lessThan(cell, ivec3(0))) || any(greaterThanEqual(cell, ivec3(size)))) break;
        uint voxel = texelFetch(packedVoxels, cell, 0).r;
        if (voxel != 0u)
        {
            float shade = axis == 0 ? 0.65 : (axis == 1 ? 1.0 : 0.8);
            return shade * mix(vec3(0.21, 0.28, 0.35), unpackUnorm4x8(voxel).rgb, 0.65);
        }
        axis = next.x <= min(next.y, next.z) ? 0 : (next.y <= next.z ? 1 : 2);
        if (start + next[axis] >= end) break;
        next[axis] += delta[axis];
        cell[axis] += stepDirection[axis];
    }
    return background;
}

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

    vec4 surface = texelFetch(normals, pixel, 0);
    if (surface.a < 0.99 || dot(surface.xyz, surface.xyz) < 0.01)
    {
        imageStore(indirectImage, pixel, vec4(0));
        return;
    }
    vec4 position = texelFetch(positionMetallic, pixel, 0);
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
