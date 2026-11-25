float voxelShadowVisibility(vec3 shadowPosition, float shadowBias)
{
    ivec2 size = textureSize(voxelShadowMap, 0);
    vec2 texel = shadowPosition.xy * vec2(size) - 0.5;
    ivec2 base = ivec2(floor(texel));
    vec2 fraction = fract(texel);
    float visibility = 0.0;
    for (int y = 0; y < 2; ++y)
    for (int x = 0; x < 2; ++x)
    {
        ivec2 pixel = clamp(base + ivec2(x, y), ivec2(0), size - 1);
        vec2 weight = mix(1.0 - fraction, fraction, vec2(x, y));
        float depth = texelFetch(voxelShadowMap, pixel, 0).r;
        visibility += weight.x * weight.y * step(shadowPosition.z - shadowBias, depth);
    }
    return visibility;
}
