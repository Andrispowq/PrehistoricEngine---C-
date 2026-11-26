#include "../water/capture.glsl"
#include "scene.glsl"
layout(location = 2) in vec3 normal_FS;
#ifndef VOXEL_RECEIVER
#include "shade.glsl"
#endif
layout(set = 1, binding = 3, std140) uniform Material
{
    layout(offset = 00) vec4 mrot;
    layout(offset = 16) vec3 colour;
    layout(offset = 32) float emission;
    layout(offset = 48) int usesNormalMap;
} material;

layout (set = 1, binding = 0) uniform sampler2D albedoMap;
layout (set = 1, binding = 1) uniform sampler2D normalMap;
layout (set = 1, binding = 2) uniform sampler2D mrotMap;

void main()
{
    vec3 albedoColour = material.colour;
    vec4 mrot = material.mrot;
    float emission = max(material.emission, 0.0);

    if (albedoColour.r == -1)
    {
        albedoColour = pow(texture(albedoMap, texture_FS).rgb, vec3(2.2));
    }

    vec4 mrotMap = texture(mrotMap, texture_FS);
    if (mrot.r == -1)
    {
        mrot.r = mrotMap.r;
    }
    if (mrot.g == -1)
    {
        mrot.g = mrotMap.g;
    }
    if (mrot.b == -1)
    {
        mrot.b = mrotMap.b;
    }
    if (mrot.a == -1)
    {
        mrot.a = mrotMap.a;
    }

    float metallic = mrot.r;
    float roughness = mrot.g;
    float occlusion = mrot.b;

    float dist = length(position_FS - (waterView.eyeAndCapture.w != 0.0 ? waterView.eyeAndCapture.xyz : cameraPosition));
    vec3 normal = normalize(normal_FS);
    if ((dist < (highDetailRange - 50)) && (material.usesNormalMap == 1) && (dot(tangent_FS, vec3(0)) != 1))
    {
        float attenuation = clamp(-dist / (highDetailRange - 50) + 1.0, 0.0, 1.0);

        vec3 bumpNormal = 2.0 * texture(normalMap, texture_FS).rbg - 1.0;
        bumpNormal = normalize(bumpNormal);

        vec3 B = normalize(cross(tangent_FS, normal));
        mat3 tbn = mat3(tangent_FS, normal, B);

        bumpNormal.xz *= attenuation;
        normal = tbn * bumpNormal;
    }

    vec3 N = normalize(normal);
    #ifdef VOXEL_RECEIVER
    outColour = vec4(position_FS, metallic);
    outBloom = vec4(N, 1);
    #else
    shadeSurface(albedoColour, metallic, roughness, occlusion, N, emission);
    outColour.a = 1.0 + float(waterView.selected);
    if (waterView.eyeAndCapture.w != 0.0) outColour.a = gl_FragCoord.z;
    #endif
}
