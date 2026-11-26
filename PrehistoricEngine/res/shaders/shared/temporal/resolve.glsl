// Canonical velocity is previous UV minus current UV, without raster jitter.
// Z/W carry current/predicted previous positive view depth; negative W rejects history.
vec4 resolveMotion(vec2 uv)
{
    ivec2 p = clamp(ivec2(uv * params.options.xy), ivec2(0), ivec2(params.options.xy) - 1);
    vec4 best = vec4(0);
    for (int i = 0; i < sampleCount(); ++i)
    {
        vec4 v = motionSample(p, i);
        if (v.z > 0 && (best.z == 0 || v.z < best.z)) best = v;
    }
    float visible = visibleDepth(p);
    if (visible > 0 && (best.z == 0 || abs(visible - best.z) > max(0.02, visible * 0.01)))
    return vec4(0, 0, visible, -1); // Unsupported/deforming surface: no inherited background motion.
    if (best.z == 0)
    {
        vec4 previous = params.reprojection * vec4((uv - params.jitter.xy) * 2 - 1, 1, 1);
        if (previous.w <= 0) return vec4(0, 0, 1e6, -1);
        best = vec4(previous.xy / previous.w * 0.5 + 0.5 - (uv - params.jitter.xy), 1e6, 1e6);
    }
    if (params.settings.w == 0) best.w = -1;
    return best;
}
