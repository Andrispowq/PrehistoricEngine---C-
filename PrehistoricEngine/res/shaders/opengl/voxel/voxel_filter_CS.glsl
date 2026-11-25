#version 450 core

layout(local_size_x = 8, local_size_y = 8) in;
layout(binding = 0, rgba32f) uniform image2D outputImage;
layout(binding = 0) uniform sampler2D indirectImage;
layout(binding = 1) uniform sampler2D positionMetallic;
layout(binding = 2) uniform sampler2D albedoRoughness;
layout(binding = 3) uniform sampler2D normals;

layout(binding = 4) uniform sampler2D emissionOcclusion;
layout(binding = 5) uniform samplerCube skyIrradiance;
uniform vec3 cameraPosition;
const float PI = 3.14159265359;
#include "shared/pbr/lighting.glsl"
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
    vec4 irradiance = filteredIndirect(pixel, size, position.xyz, normalize(receiver.xyz), traceStride, voxelSize, volumeMin, volumeExtent);
    if (irradiance.a == 0.0) return;
    vec4 surface = texelFetch(albedoRoughness, pixel, 0);
    vec3 normal = normalize(receiver.xyz);
    vec3 V = normalize(cameraPosition - position.xyz);
    float metallic = clamp(position.w, 0.0, 1.0);
    vec3 F = FresnelSchlickRoughness(max(dot(normal, V), 0.0), mix(vec3(0.04), surface.rgb, metallic), surface.a);
    // PBR receivers record 1 + material AO. Water uses its own planar lighting.
    float encodedAO = texelFetch(emissionOcclusion, pixel, 0).a;
    vec4 scene = imageLoad(outputImage, pixel);
    if (encodedAO >= 1.0)
    {
        vec3 response = (1.0 - F) * (1.0 - metallic) * surface.rgb * (encodedAO - 1.0);
        scene.rgb += response * diffuseTraceCorrection(irradiance, textureLod(skyIrradiance, normal, 0.0).rgb, giStrength);
    }
    else
    scene.rgb += surface.rgb * irradiance.rgb * giStrength;
    imageStore(outputImage, pixel, scene);
}
