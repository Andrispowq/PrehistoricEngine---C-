vec4 filteredIndirect(ivec2 pixel, ivec2 size, vec3 position, vec3 normal, int traceStride, float voxelSize, vec3 volumeMin, float volumeExtent)
{
    bool halfResolution = traceStride == 2;
    ivec2 center = pixel / traceStride;
    vec4 indirect = texelFetch(indirectImage, center, 0);
    if (!halfResolution && indirect.a == 0.0) return vec4(0);

    vec3 uvw = (position - volumeMin) / volumeExtent;
    #ifdef inside
    if (halfResolution && !inside(uvw)) return vec4(0);
    #else
    if (halfResolution && (any(lessThan(uvw, vec3(0))) || any(greaterThanEqual(uvw, vec3(1))))) return vec4(0);
    #endif
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
        vec3 offset = (texelFetch(positionMetallic, samplePixel, 0).xyz - position) / voxelSize;
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

    return total < 0.001 ? vec4(0) : vec4(sum / total, 1.0);
}

// Replace the existing diffuse sky term only where the trace yielded a receiver.
vec3 diffuseTraceCorrection(vec4 traced, vec3 skyIrradiance, float strength)
{
    return traced.a * (strength * traced.rgb - min(strength, 1.0) * skyIrradiance);
}
