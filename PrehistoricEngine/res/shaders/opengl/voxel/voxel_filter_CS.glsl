#version 450 core

layout(local_size_x = 8, local_size_y = 8) in;
layout(binding = 0, rgba32f) uniform image2D outputImage;
layout(binding = 0) uniform sampler2D indirectImage;
layout(binding = 1) uniform sampler2D positionMetallic;
layout(binding = 2) uniform sampler2D albedoRoughness;
layout(binding = 3) uniform sampler2D normals;

uniform float giStrength;
uniform float voxelSize;

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);
    if (any(greaterThanEqual(pixel, size))) return;

    vec4 indirect = texelFetch(indirectImage, pixel, 0);
    if (indirect.a == 0.0) return;

    vec4 position = texelFetch(positionMetallic, pixel, 0);
    vec3 normal = normalize(texelFetch(normals, pixel, 0).xyz);
    vec3 sum = 4.0 * indirect.rgb;
    float total = 4.0;
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
        {
            if (x == 0 && y == 0) continue;
            ivec2 tap = pixel + 2 * ivec2(x, y);
            if (any(lessThan(tap, ivec2(0))) || any(greaterThanEqual(tap, size))) continue;
            vec4 sampleValue = texelFetch(indirectImage, tap, 0);
            if (sampleValue.a == 0.0) continue;

            vec3 offset = (texelFetch(positionMetallic, tap, 0).xyz - position.xyz) / voxelSize;
            vec3 tapNormal = normalize(texelFetch(normals, tap, 0).xyz);
            float normalWeight = smoothstep(0.85, 0.995, dot(normal, tapNormal));
            float planeWeight = 1.0 - smoothstep(0.04, 0.22, abs(dot(normal, offset)));
            float distanceWeight = 1.0 - smoothstep(0.35, 1.25, length(offset));
            float spatialWeight = (x == 0 || y == 0) ? 2.0 : 1.0;
            float weight = spatialWeight * normalWeight * planeWeight * distanceWeight;
            sum += sampleValue.rgb * weight;
            total += weight;
        }

    vec3 albedo = texelFetch(albedoRoughness, pixel, 0).rgb;
    vec4 scene = imageLoad(outputImage, pixel);
    scene.rgb += (sum / total) * albedo * (1.0 - clamp(position.w, 0.0, 1.0)) * giStrength;
    imageStore(outputImage, pixel, scene);
}
