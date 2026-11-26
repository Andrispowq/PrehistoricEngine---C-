#version 450 core

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout(binding = 0) uniform sampler3D radianceVolume;
layout(binding = 0, r32ui) uniform readonly uimage3D bounceSurfaces;
layout(binding = 1, rgba16f) uniform image3D accumulatedRadiance;
layout(binding = 2, rgba16f) uniform writeonly image3D nextRadiance;
uniform bool writeNext;

layout(binding = 5) uniform samplerCube coneEnvironment;
uniform bool useEnvironment;
#include "opengl/voxel/voxel_cones.glsl"
#include "shared/voxel/voxel_cones.glsl"
#include "opengl/voxel/voxel_surface.glsl"

void main()
{
    ivec3 cell = ivec3(gl_GlobalInvocationID);
    uint surface = imageLoad(bounceSurfaces, cell).r;
    if (surface == 0u)
    {
        if (writeNext) imageStore(nextRadiance, cell, vec4(0));
        return;
    }

    vec3 reflectance = unpackBounceReflectance(surface);
    vec3 bounce = vec3(0);
    if (any(greaterThan(reflectance, vec3(0))))
    {
        float cellSize = 1.0 / float(imageSize(bounceSurfaces).x);
        vec3 origin = (vec3(cell) + 0.5) * cellSize;
        bounce = reflectance * traceHemisphere(origin, unpackBounceNormal(surface), cellSize);
        vec4 accumulated = imageLoad(accumulatedRadiance, cell);
        imageStore(accumulatedRadiance, cell, vec4(accumulated.rgb + bounce, accumulated.a));
    }
    if (writeNext) imageStore(nextRadiance, cell, vec4(bounce, 1.0));
}
