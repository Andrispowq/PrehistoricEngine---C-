vec3 ToneMapHDR(vec3 radiance, float exposure, float gamma)
{
    vec3 mapped = 1.0 - exp(-max(radiance, vec3(0.0)) * exposure);
    return pow(mapped, vec3(1.0 / gamma));
}

// The scene pass writes alpha 2 for selected geometry and at most 1 elsewhere.
// MSAA resolves partial coverage before this display-space edge filter.
vec3 selectionOutline(sampler2D scene, ivec2 pixel, vec3 colour)
{
    ivec2 last = textureSize(scene, 0) - 1;
    float center = clamp(texelFetch(scene, pixel, 0).a - 1.0, 0.0, 1.0);
    if (center >= 1.0) return colour;
    float coverage = center;
    const ivec2 offsets[12] = ivec2[](ivec2(-1, 0), ivec2(1, 0), ivec2(0, -1), ivec2(0, 1),
            ivec2(-2, 0), ivec2(2, 0), ivec2(0, -2), ivec2(0, 2),
            ivec2(-1, -1), ivec2(1, -1), ivec2(-1, 1), ivec2(1, 1));
    for (int i = 0; i < offsets.length(); ++i) {
        ivec2 samplePixel = clamp(pixel + offsets[i], ivec2(0), last);
        coverage = max(coverage, texelFetch(scene, samplePixel, 0).a - 1.0);
    }
    return mix(colour, vec3(1.0, 0.62, 0.10), clamp(coverage - center, 0.0, 1.0));
}
