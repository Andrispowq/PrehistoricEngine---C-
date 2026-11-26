bool inside(vec3 p)
{
    return all(greaterThanEqual(p, vec3(0))) && all(lessThan(p, vec3(1)));
}

vec3 filterHitRadiance(ivec3 cell, vec3 start, vec3 direction, float distance)
{
    // Find the entry face and interpolate only on its voxel slice.
    vec3 ray = mix(direction, vec3(1e-6), lessThan(abs(direction), vec3(1e-6)));
    vec3 entry = (vec3(cell) + step(ray, vec3(0)) - start) / ray;
    int axis = entry.x >= max(entry.y, entry.z) ? 0 : (entry.y >= entry.z ? 1 : 2);
    int u = (axis + 1) % 3;
    int v = (axis + 2) % 3;
    vec3 hit = start + ray * distance - 0.5;
    ivec3 base = cell;
    base[u] = int(floor(hit[u]));
    base[v] = int(floor(hit[v]));
    vec2 fraction = fract(vec2(hit[u], hit[v]));
    ivec3 size = ivec3(voxelGridSize());
    vec3 radiance = vec3(0);
    float total = 0.0;
    for (int y = 0; y < 2; ++y)
    for (int x = 0; x < 2; ++x)
    {
        ivec3 tap = base;
        tap[u] += x;
        tap[v] += y;
        if (any(lessThan(tap, ivec3(0))) || any(greaterThanEqual(tap, size))) continue;
        vec4 sampleValue = baseSample(tap);
        vec2 weight = mix(1.0 - fraction, fraction, vec2(x, y));
        float w = weight.x * weight.y * sampleValue.a;
        radiance += sampleValue.rgb * w;
        total += w;
    }
    return total > 0.0 ? radiance / total : baseRadiance(cell);
}
