vec3 compressHDR(vec3 c) { return max(c, vec3(0)) / (1 + max(max(c.r, c.g), c.b)); }
vec3 expandHDR(vec3 c) { return c / max(1e-4, 1 - max(max(c.r, c.g), c.b)); }
vec2 blurVelocity(vec4 m)
{
    if (m.w <= 0) return vec2(0);
    vec2 v = m.xy * params.options.xy * params.settings.x * 0.5;
    return v * min(1.0, params.settings.y / max(length(v), 1e-5));
}
vec4 temporalAA(vec2 uv, bool diagnostic)
{
    ivec2 size = textureSize(sourceImage, 0);
    ivec2 p = clamp(ivec2((uv + params.jitter.xy) * size), ivec2(0), size - 1);
    vec4 current = texture(sourceImage, uv + params.jitter.xy);
    vec4 motion = texelFetch(motionImage, p, 0);
    bool surfaceValid = motion.w > 0;
    // Foreground dilation prevents background velocity from dragging thin edges.
    for (int y = -1; y <= 1; ++y) for (int x = -1; x <= 1; ++x)
    {
        vec4 candidate = texelFetch(motionImage, clamp(p + ivec2(x, y), ivec2(0), size - 1), 0);
        if (candidate.z > 0 && candidate.z < motion.z) motion = candidate;
    }
    vec2 previousUV = uv + motion.xy;
    float weight = 0;
    vec3 history = current.rgb;
    if (surfaceValid && params.settings.w > 0 && motion.w > 0 && all(greaterThanEqual(previousUV, vec2(0))) &&
        all(lessThan(previousUV, vec2(1))))
    {
        float oldDepth = texture(previousMotion, previousUV + params.jitter.zw).z;
        if (abs(oldDepth - motion.w) < max(0.02, motion.w * 0.01))
        {
            vec3 mean = vec3(0), second = vec3(0), lo = vec3(1), hi = vec3(0);
            for (int y = -1; y <= 1; ++y) for (int x = -1; x <= 1; ++x)
            {
                vec3 c = compressHDR(texelFetch(sourceImage, clamp(p + ivec2(x, y), ivec2(0), size - 1), 0).rgb);
                mean += c; second += c * c; lo = min(lo, c); hi = max(hi, c);
            }
            mean /= 9; second /= 9;
            vec3 sigma = sqrt(max(vec3(0), second - mean * mean));
            vec3 old = compressHDR(texture(historyImage, previousUV).rgb);
            vec3 clipped = clamp(old, max(lo, mean - 1.25 * sigma), min(hi, mean + 1.25 * sigma));
            // React to shading changes (SSR, emission and lighting) even when geometry is stationary.
            float change = length(old - compressHDR(current.rgb));
            weight = mix(0.9, 0.2, clamp(change * 4, 0, 1));
            weight *= 1 - clamp(length(motion.xy * size) / 128, 0, 0.75);
            history = expandHDR(clipped);
        }
    }
    if (diagnostic && params.options.w == 3) return vec4(vec3(weight == 0 ? 1 : 0), current.a);
    if (diagnostic && params.options.w == 4) return vec4(vec3(weight), current.a);
    return vec4(mix(current.rgb, history, weight), current.a);
}
vec4 motionBlur(vec2 uv)
{
    vec4 center = texture(sourceImage, uv);
    vec2 motionOffset = params.options.z > 0 ? params.jitter.xy : vec2(0);
    vec4 m = texture(motionImage, uv + motionOffset);
    if (params.options.w == 1) return vec4(0.5 + m.xy * 8, 0.5, center.a);
    if (params.options.w == 2) return vec4(m.w > 0 ? vec3(0, 1, 0) : vec3(1, 0, 0), center.a);
    if (m.w <= 0) return center;
    vec2 dominant = texture(tileImage, uv).xy;
    vec2 own = blurVelocity(m);
    if (length(dominant) < 0.5) return center;
    vec3 sum = center.rgb;
    float total = 1;
    int count = int(params.settings.z);
    for (int i = 0; i < count; ++i)
    {
        float t = (float(i) + 0.5) / count * 2 - 1;
        vec2 v = i % 2 == 0 && length(own) > 0.5 ? own : dominant;
        vec2 q = uv + t * v / params.options.xy;
        if (any(lessThan(q, vec2(0))) || any(greaterThanEqual(q, vec2(1)))) continue;
        vec4 sampleMotion = texture(motionImage, q + motionOffset);
        if (sampleMotion.w <= 0) continue;
        float distancePixels = abs(t) * length(v);
        float reach = length(blurVelocity(sampleMotion));
        bool foreground = sampleMotion.z < m.z - max(0.02, m.z * 0.01);
        float w = foreground ? clamp(reach - distancePixels + 1, 0, 1) : clamp(length(own) - distancePixels + 1, 0, 1);
        if (sampleMotion.z > m.z * 1.01 && length(own) < distancePixels) w = 0;
        sum += texture(sourceImage, q).rgb * w; total += w;
    }
    return vec4(sum / total, center.a);
}
vec4 tileMaximum(ivec2 tile)
{
    vec2 best = vec2(0);
    ivec2 size = textureSize(motionImage, 0);
    for (int y = 0;y < 16;++y) for (int x = 0;x < 16;++x)
    {
        vec2 v = blurVelocity(texelFetch(motionImage, min(tile * 16 + ivec2(x, y), size - 1), 0));
        if (dot(v, v) > dot(best, best)) best = v;
    }
    return vec4(best, 0, 0);
}
vec4 neighborMaximum(ivec2 tile)
{
    vec2 best = vec2(0);
    ivec2 size = textureSize(tileImage, 0);
    // Maximum supported radius is 64 px, i.e. four 16-pixel tiles.
    int radius = int(ceil(params.settings.y / 16));
    for (int y = -radius;y <= radius;++y) for (int x = -radius;x <= radius;++x)
    {
        vec2 v = texelFetch(tileImage, clamp(tile + ivec2(x, y), ivec2(0), size - 1), 0).xy;
        if (dot(v, v) > dot(best, best)) best = v;
    }
    return vec4(best, 0, 0);
}
