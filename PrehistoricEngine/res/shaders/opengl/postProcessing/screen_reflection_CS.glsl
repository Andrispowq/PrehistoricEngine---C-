#version 450 core
layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba32f, binding = 0) uniform writeonly image2D outputImage;
layout(r16f, binding = 1) uniform writeonly image2D confidenceImage;
layout(binding = 0) uniform sampler2D screenRadiance;
layout(binding = 1) uniform sampler2D positionMetallic;
layout(binding = 2) uniform sampler2D albedoRoughness;
layout(binding = 3) uniform sampler2D normals;
layout(binding = 4) uniform sampler2D emissionOcclusion;
layout(binding = 5) uniform samplerCube prefilterMap;
layout(binding = 6) uniform sampler2D brdfLUT;
uniform mat4 m_view, m_projection;
uniform vec3 cameraPosition;
uniform float maxReflectionLOD;
const float PI = 3.14159265359;
#include "shared/pbr/lighting.glsl"
#include "shared/postProcessing/screen_reflection.glsl"
void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(pixel, imageSize(outputImage)))) return;
    vec4 n = texelFetch(normals, pixel, 0);
    vec4 surface = texelFetch(albedoRoughness, pixel, 0);
    float encodedAO = texelFetch(emissionOcclusion, pixel, 0).a;
    float confidence = 0.0;
    if (n.a >= 0.99 && encodedAO >= 1.0 && surface.a < 0.55)
    {
        vec4 pos = texelFetch(positionMetallic, pixel, 0);
        vec3 N = normalize(n.xyz), V = normalize(cameraPosition - pos.xyz);
        vec3 R = reflect(-V, N);
        vec4 hit = traceScreenReflection(pos.xyz, N, R, surface.a);
        confidence = hit.a;
        if (confidence > 0.0)
        {
            float NoV = max(dot(N, V), 0.0);
            vec3 F = FresnelSchlickRoughness(NoV, mix(vec3(0.04), surface.rgb, pos.w), surface.a);
            vec2 brdf = textureLod(brdfLUT, vec2(max(NoV, 0.01), surface.a), 0).rg;
            vec3 sky = textureLod(prefilterMap, R, surface.a * maxReflectionLOD).rgb;
            float ao = encodedAO >= 3.0 ? encodedAO - 3.0 : 1.0;
            vec4 colour = texelFetch(screenRadiance, pixel, 0);
            colour.rgb += confidence * ao * (F * brdf.x + brdf.y) * (hit.rgb - sky);
            imageStore(outputImage, pixel, colour);
        }
    }
    imageStore(confidenceImage, pixel, vec4(confidence));
}
