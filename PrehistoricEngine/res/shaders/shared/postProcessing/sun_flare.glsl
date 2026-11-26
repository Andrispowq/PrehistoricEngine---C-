// Seven additive sprites, from the source halo through the optical center to the ghost images.
vec3 sunLensFlare(vec2 uv, vec4 sun, vec2 footprint)
{
    if (sun.z <= 0.0) return vec3(0);
    vec2 aspect = vec2(footprint.y / footprint.x, 1.0);
    vec2 axis = (0.5 - sun.xy) * aspect;
    float distanceToCenter = length(axis);
    vec2 rotation = distanceToCenter > 0.0001 ? axis / distanceToCenter : vec2(1, 0);
    mat2 rotate = mat2(rotation.x, -rotation.y, rotation.y, rotation.x);
    vec2 fromSun = rotate * ((uv - sun.xy) * aspect);
    float reach = 0.12 * sun.w;
    if (abs(fromSun.y) > reach || fromSun.x < -reach || fromSun.x > 1.91 * distanceToCenter + reach)
    return vec3(0);
    const float offset[7] = float[](0.0, 0.32, 0.61, 0.85, 1.24, 1.55, 1.91);
    const float radius[7] = float[](0.12, 0.034, 0.048, 0.025, 0.045, 0.089, 0.12);
    const vec3 tint[7] = vec3[](vec3(1.0, .75, .42), vec3(.23, .37, .31), vec3(.28, .18, .08),
            vec3(.13, .29, .38), vec3(.20, .28, .12), vec3(.14, .22, .32), vec3(.24, .13, .10));
    vec3 ghosts = vec3(0);
    for (int i = 0; i < 7; ++i)
    {
        vec2 local = (fromSun - vec2(distanceToCenter * offset[i], 0)) / (radius[i] * sun.w);
        if (max(abs(local.x), abs(local.y)) >= 1.0) continue;
        vec2 tile = local * 0.5 + 0.5;
        ghosts += tint[i] * textureLod(flareAtlas, vec2((float(i) + tile.x) / 7.0, tile.y), 0.0).r;
    }
    if (dot(ghosts, ghosts) == 0.0) return ghosts;
    // Sample the visible sky near the source. All ghosts share that visibility.
    vec2 tap = footprint;
    float visible = skyCoverage(sun.xy);
    visible += skyCoverage(sun.xy + vec2(tap.x, 0)) + skyCoverage(sun.xy - vec2(tap.x, 0));
    visible += skyCoverage(sun.xy + vec2(0, tap.y)) + skyCoverage(sun.xy - vec2(0, tap.y));
    vec2 edge = min(sun.xy, 1.0 - sun.xy);
    float fade = smoothstep(0.0, 0.12, min(edge.x, edge.y)) / (1.0 + 6.0 * distanceToCenter);
    float visibility = visible / 5.0;
    // Set sprite radiance before exposure so the ghosts also register against daylight sky.
    return ghosts * (5.0 * visibility * fade * sun.z);
}
