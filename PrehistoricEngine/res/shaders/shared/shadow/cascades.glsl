// The first four taps are spread over the whole disk for the volume path.
const vec2 ShadowDisk[12] = vec2[](
        vec2(-0.58, -0.40), vec2(0.40, -0.58), vec2(0.58, 0.40), vec2(-0.40, 0.58),
        vec2(-0.89, 0.17), vec2(-0.12, -0.92), vec2(0.90, -0.21), vec2(0.17, 0.90),
        vec2(-0.27, -0.10), vec2(0.10, -0.27), vec2(0.27, 0.10), vec2(-0.10, 0.27)
);
const float MaxShadowFilterRadius = 20.0;

float shadowDepthAt(ivec2 pixel, int layer, ivec2 size)
{
    return texelFetch(shadowMap, ivec3(clamp(pixel, ivec2(0), size - 1), layer), 0).r;
}

float filteredShadowTap(vec2 texel, float receiverDepth, vec2 depthSlope, int layer, ivec2 size)
{
    ivec2 base = ivec2(floor(texel));
    vec2 fraction = fract(texel);
    float visibility = 0.0;
    for (int y = 0; y < 2; ++y)
    {
        for (int x = 0; x < 2; ++x)
        {
            vec2 tap = vec2(x, y);
            vec2 weight = mix(1.0 - fraction, fraction, tap);
            float depth = receiverDepth + dot(depthSlope, tap - fraction);
            if (depth <= shadowDepthAt(base + ivec2(x, y), layer, size))
            visibility += weight.x * weight.y;
        }
    }
    return visibility;
}

float cascadeVisibility(vec3 position, vec3 normal, int layer, float biasInTexels, bool volume)
{
    mat4 lightMatrix = m_LightSpace[layer];
    vec3 coord = (lightMatrix * vec4(position, 1.0)).xyz * 0.5 + 0.5;
    if (any(lessThan(coord, vec3(0.0))) || any(greaterThan(coord, vec3(1.0))))
    return 1.0;

    ivec2 size = textureSize(shadowMap, 0).xy;
    vec2 texel = coord.xy * vec2(size) - 0.5;
    vec3 rowX = vec3(lightMatrix[0].x, lightMatrix[1].x, lightMatrix[2].x);
    vec3 rowY = vec3(lightMatrix[0].y, lightMatrix[1].y, lightMatrix[2].y);
    vec3 rowZ = vec3(lightMatrix[0].z, lightMatrix[1].z, lightMatrix[2].z);
    float texelWorldSize = 2.0 / (length(rowX) * float(size.x));
    float depthPerWorldUnit = 0.5 * length(rowZ);
    float depthPerTexel = depthPerWorldUnit * texelWorldSize;
    float receiverDepth = coord.z - biasInTexels * depthPerTexel;

    // Account for the receiver plane so a wide kernel doesn't self-shadow a sloped surface.
    vec2 depthSlope = vec2(0.0);
    if (!volume)
    {
        float normalZ = dot(normal, normalize(rowZ));
        if (abs(normalZ) > 0.1)
        {
            vec2 normalXY = vec2(dot(normal, normalize(rowX)), dot(normal, normalize(rowY)));
            depthSlope = clamp(-normalXY / normalZ, vec2(-4.0), vec2(4.0)) * depthPerTexel;
        }
    }

    float center = filteredShadowTap(texel, receiverDepth, depthSlope, layer, size);
    if (shadowSunAngularRadius <= 0.0) return center;

    float sunSlope = tan(shadowSunAngularRadius);
    // Search a bounded neighborhood for blockers, then scale the penumbra
    // by the measured world-space separation between blocker and receiver.
    float searchRadius = clamp(sunSlope * 40.0 / texelWorldSize, 1.0, MaxShadowFilterRadius);
    float blockerSeparation = 0.0;
    int blockers = 0;
    for (int i = -1; i < (volume ? 4 : 12); ++i)
    {
        vec2 sampleOffset = i < 0 ? vec2(0.0) : ShadowDisk[i] * searchRadius;
        ivec2 pixel = ivec2(floor(texel + sampleOffset + 0.5));
        float sampleDepth = receiverDepth + dot(depthSlope, vec2(pixel) - texel);
        float separation = sampleDepth - shadowDepthAt(pixel, layer, size);
        if (separation > 0.0)
        {
            blockerSeparation += separation;
            ++blockers;
        }
    }
    if (blockers == 0) return center;

    float separationWorld = blockerSeparation / (float(blockers) * depthPerWorldUnit);
    float radius = min(sunSlope * separationWorld / texelWorldSize, MaxShadowFilterRadius);
    if (radius < 0.75) return center;

    float visibility = center;
    int taps = volume ? 4 : 12;
    for (int i = 0; i < taps; ++i)
    {
        vec2 offset = ShadowDisk[i] * radius;
        if (volume)
        {
            ivec2 pixel = ivec2(floor(texel + offset + 0.5));
            float depth = receiverDepth + dot(depthSlope, vec2(pixel) - texel);
            visibility += float(depth <= shadowDepthAt(pixel, layer, size));
        }
        else
        {
            visibility += filteredShadowTap(texel + offset, receiverDepth + dot(depthSlope, offset),
                    depthSlope, layer, size);
        }
    }
    return visibility / float(taps + 1);
}

float cascadeSunVisibility(vec3 position, vec3 normal, float biasInTexels, bool volume)
{
    float depth = -(shadowView * vec4(position, 1.0)).z;
    if (depth < shadowNearPlane) return 1.0;

    int layer = 0;
    while (layer < cascadeCount && depth >= cascadeSplit(layer))
    ++layer;

    float visibility = cascadeVisibility(position, normal, layer, biasInTexels, volume);
    if (layer == cascadeCount) return visibility;

    float near = layer == 0 ? shadowNearPlane : cascadeSplit(layer - 1);
    float far = cascadeSplit(layer);
    float blendStart = mix(far, near, cascadeBlendFraction);
    if (depth <= blendStart) return visibility;
    return mix(visibility, cascadeVisibility(position, normal, layer + 1, biasInTexels, volume),
            smoothstep(blendStart, far, depth));
}
