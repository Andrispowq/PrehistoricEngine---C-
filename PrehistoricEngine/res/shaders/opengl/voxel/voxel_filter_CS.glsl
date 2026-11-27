#version 450 core

layout(local_size_x = 8, local_size_y = 8) in;
layout(binding = 0, rgba32f) uniform image2D outputImage;
layout(binding = 0) uniform sampler2D indirectImage;
layout(binding = 1) uniform sampler2D positionMetallic;
layout(binding = 2) uniform sampler2D albedoRoughness;
layout(binding = 3) uniform sampler2D normals;

layout(binding = 4) uniform sampler2D emissionOcclusion;
layout(binding = 5) uniform samplerCube skyIrradiance;
layout(binding = 6) uniform sampler3D radianceVolume;
layout(binding = 7) uniform samplerCube prefilterMap;
layout(binding = 15) uniform sampler2D brdfLUT;
layout(r16f, binding = 1) uniform readonly image2D screenConfidence;
uniform bool useScreenReflections;
uniform float maxReflectionLOD;
uniform vec3 volumeMin;
uniform float volumeExtent;
#include "opengl/voxel/voxel_cones.glsl"
#include "shared/voxel/reflection.glsl"
layout(std430, binding = 6) readonly buffer VoxelCascadeData { uint cascadeWords[]; };
#define voxelOcclusionMin volumeMin
#define voxelOcclusionExtent volumeExtent
#include "shared/voxel/cascades.glsl"
uniform vec3 cameraPosition;
const float PI = 3.14159265359;
#include "shared/pbr/lighting.glsl"
uniform float giStrength;
uniform bool terrainReceivesGI;
uniform float voxelSize;
uniform int traceStride;

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

    vec4 surface = texelFetch(albedoRoughness, pixel, 0);
    vec3 normal = normalize(receiver.xyz);
    vec3 V = normalize(cameraPosition - position.xyz);
    float metallic = clamp(position.w, 0.0, 1.0);
    vec3 F = FresnelSchlickRoughness(max(dot(normal, V), 0.0), mix(vec3(0.04), surface.rgb, metallic), surface.a);
    // Meshes store 1 + AO, terrain stores 3 + AO and applies AO to both ambient lobes.
    float encodedAO = texelFetch(emissionOcclusion, pixel, 0).a;
    if (encodedAO >= 3.0 && !terrainReceivesGI) return;
    if (encodedAO < 0.0) return; // Water handles voxel reflections before Fresnel composition.
    vec4 scene = imageLoad(outputImage, pixel);
    if (encodedAO >= 1.0)
    {
        bool terrain = encodedAO >= 3.0;
        float ao = encodedAO - (terrain ? 3.0 : 1.0);
        vec3 origin = (position.xyz - volumeMin) / volumeExtent;
        float weight = voxelReflectionWeight(metallic, surface.a, giStrength);
        if (useScreenReflections) weight *= 1.0 - imageLoad(screenConfidence, pixel).r;
        if (weight > 0.0 && inside(origin))
        {
            vec3 R = reflect(-V, normal);
            vec4 reflected = traceReflection(origin, normal, R, surface.a);
            vec3 sky = textureLod(prefilterMap, R, surface.a * maxReflectionLOD).rgb;
            vec2 brdf = textureLod(brdfLUT, vec2(max(dot(normal, V), 0.01), surface.a), 0.0).rg;
            scene.rgb += reflectionCorrection(reflected, sky, F * brdf.x + brdf.y, weight * (terrain ? ao : 1.0));
        }
        vec3 response = (1.0 - F) * (1.0 - metallic) * surface.rgb * ao;
        scene.rgb += response * diffuseTraceCorrection(irradiance, textureLod(skyIrradiance, normal, 0.0).rgb, giStrength);
    }
    else
    scene.rgb += surface.rgb * irradiance.rgb * giStrength;
    imageStore(outputImage, pixel, scene);
}
