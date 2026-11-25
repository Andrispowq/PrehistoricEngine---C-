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

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);
    if (any(greaterThanEqual(pixel, size))) return;

    vec4 receiver = texelFetch(normals, pixel, 0);
    if (receiver.a < 0.99 || dot(receiver.xyz, receiver.xyz) < 0.01) return;
    bool halfResolution = traceStride == 2;
    ivec2 center = pixel / traceStride;
    vec4 indirect = texelFetch(indirectImage, center, 0);
    if (!halfResolution && indirect.a == 0.0) return;

    vec4 position = texelFetch(positionMetallic, pixel, 0);
    vec3 uvw = (position.xyz - volumeMin) / volumeExtent;
    if (halfResolution && (any(lessThan(uvw, vec3(0))) || any(greaterThanEqual(uvw, vec3(1))))) return;
    vec3 normal = normalize(receiver.xyz);
    vec3 sum = halfResolution ? vec3(0.0) : 4.0 * indirect.rgb;
    float total = halfResolution ? 0.0 : 4.0;
    ivec2 indirectSize = textureSize(indirectImage, 0);
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
        {
            if (!halfResolution && x == 0 && y == 0) continue;
            ivec2 tap = center + (halfResolution ? 1 : 2) * ivec2(x, y);
            if (any(lessThan(tap, ivec2(0))) || any(greaterThanEqual(tap, indirectSize))) continue;
            vec4 sampleValue = texelFetch(indirectImage, tap, 0);
            if (sampleValue.a == 0.0) continue;

            ivec2 samplePixel = min(tap * traceStride + traceStride / 2, size - 1);
            vec3 offset = (texelFetch(positionMetallic, samplePixel, 0).xyz - position.xyz) / voxelSize;
            vec3 tapNormal = normalize(texelFetch(normals, samplePixel, 0).xyz);
            float normalWeight = smoothstep(0.85, 0.995, dot(normal, tapNormal));
            float planeWeight = 1.0 - smoothstep(0.04, 0.22, abs(dot(normal, offset)));
            float distanceWeight = 1.0 - smoothstep(0.35, 1.25, length(offset));
            vec2 pixelDistance = vec2(samplePixel - pixel);
            float spatialWeight = halfResolution ? 1.0 / (1.0 + dot(pixelDistance, pixelDistance))
                                                  : ((x == 0 || y == 0) ? 2.0 : 1.0);
            float weight = spatialWeight * normalWeight * planeWeight * distanceWeight;
            sum += sampleValue.rgb * weight;
            total += weight;
        }

    if (total < 0.001) return;
    vec3 albedo = texelFetch(albedoRoughness, pixel, 0).rgb;
    vec4 scene = imageLoad(outputImage, pixel);
    scene.rgb += (sum / total) * albedo * (1.0 - clamp(position.w, 0.0, 1.0)) * giStrength;
    imageStore(outputImage, pixel, scene);
}
