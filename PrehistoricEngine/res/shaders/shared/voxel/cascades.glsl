// Read-only snapshot shared by both APIs. All traversal distances are world units.
// cascadeWords is a uint SSBO, with the VoxelCascadeSnapshot header at word zero.
int vxCascadeCount() { return int(cascadeWords[0]); }
vec4 vxCascadeBounds(int c)
{
    int b = 4 + 4 * c;
    return vec4(uintBitsToFloat(cascadeWords[b]), uintBitsToFloat(cascadeWords[b + 1]),
            uintBitsToFloat(cascadeWords[b + 2]), uintBitsToFloat(cascadeWords[b + 3]));
}
uvec4 vxCascadeGrid(int c)
{
    int b = 20 + 4 * c;
    return uvec4(cascadeWords[b], cascadeWords[b + 1], cascadeWords[b + 2], cascadeWords[b + 3]);
}
float vxCascadeCell(int c) { return vxCascadeBounds(c).w / float(vxCascadeGrid(c).x); }
bool vxCascadeContains(int c, vec3 p)
{
    vec4 b = vxCascadeBounds(c);
    vec3 v = (p - b.xyz) / b.w;
    return all(greaterThanEqual(v, vec3(0))) && all(lessThan(v, vec3(1)));
}
int vxCascadeAt(vec3 p, float diameter)
{
    for (int c = 0; c < vxCascadeCount(); ++c)
    if (vxCascadeContains(c, p) && (diameter < 2.0 * vxCascadeCell(c) || c + 1 == vxCascadeCount())) return c;
    // Wide cones may still overlap the outer volume.
    return vxCascadeContains(vxCascadeCount() - 1, p) ? vxCascadeCount() - 1 : -1;
}
int vxCascadeMipOffset(int n, int level) { return (8 * (n * n * n - ((n * n * n) >> (3 * level)))) / 7; }
int vxCascadeIndex(ivec3 p, int n) { return p.x + n * (p.y + n * p.z); }
vec4 vxCascadeFlux(int address)
{
    return vec4(unpackHalf2x16(cascadeWords[address]), unpackHalf2x16(cascadeWords[address + 1]));
}
vec4 vxCascadeLevel(int c, vec3 p, int level, int face)
{
    uvec4 grid = vxCascadeGrid(c);
    int n = int(grid.x), width = n >> level;
    int offset = vxCascadeMipOffset(n, level);
    int base = int(grid.z) + 2 * offset;
    if (face >= 0)
    {
        int pyramid = (8 * n * n * n - 1) / 7;
        base = int(grid.w) + 2 * (face * (pyramid - n * n * n) + offset - n * n * n);
    }
    vec4 bounds = vxCascadeBounds(c);
    vec3 at = clamp((p - bounds.xyz) / bounds.w * float(width) - 0.5, vec3(0), vec3(width - 1));
    ivec3 cell = ivec3(at);
    vec3 f = fract(at);
    vec4 sum = vec4(0);
    for (int i = 0; i < 8; ++i)
    {
        ivec3 corner = ivec3(i & 1, (i >> 1) & 1, i >> 2);
        vec3 w = mix(1.0 - f, f, vec3(corner));
        sum += w.x * w.y * w.z * vxCascadeFlux(base + 2 * vxCascadeIndex(min(cell + corner, ivec3(width - 1)), width));
    }
    return sum;
}
vec4 vxCascadeDirectional(int c, vec3 p, vec3 ray, int level)
{
    if (level == 0 || cascadeWords[1] == 0u) return vxCascadeLevel(c, p, level, -1);
    vec3 w = ray * ray;
    return w.x * vxCascadeLevel(c, p, level, ray.x >= 0 ? 0 : 1) +
    w.y * vxCascadeLevel(c, p, level, ray.y >= 0 ? 2 : 3) +
    w.z * vxCascadeLevel(c, p, level, ray.z >= 0 ? 4 : 5);
}
vec4 vxCascadeSample(int c, vec3 p, vec3 ray, float diameter)
{
    float lod = clamp(log2(max(1.0, diameter / vxCascadeCell(c))), 0.0, float(findMSB(vxCascadeGrid(c).x)));
    int low = int(lod), high = min(low + 1, findMSB(vxCascadeGrid(c).x));
    return mix(vxCascadeDirectional(c, p, ray, low), vxCascadeDirectional(c, p, ray, high), fract(lod));
}
vec4 vxCascadeSampleBlended(vec3 p, vec3 ray, float diameter)
{
    int c = vxCascadeAt(p, diameter);
    if (c < 0) return vec4(0);
    vec4 value = vxCascadeSample(c, p, ray, diameter);
    if (c + 1 < vxCascadeCount() && vxCascadeContains(c + 1, p))
    {
        vec4 b = vxCascadeBounds(c);
        vec3 edge = min(p - b.xyz, b.xyz + b.w - p);
        float margin = max(2.0 * vxCascadeCell(c), diameter);
        float edgeBlend = 1.0 - smoothstep(0.0, margin, min(edge.x, min(edge.y, edge.z)));
        float lodBlend = smoothstep(vxCascadeCell(c), 2.0 * vxCascadeCell(c), diameter);
        value = mix(value, vxCascadeSample(c + 1, p, ray, diameter), max(edgeBlend, lodBlend));
    }
    return value;
}
vec2 vxCascadeIntersection(vec3 p, vec3 ray, vec4 b)
{
    vec3 a = (b.xyz - p) / ray, z = (b.xyz + b.w - p) / ray;
    vec3 lo = min(a, z), hi = max(a, z);
    return vec2(max(lo.x, max(lo.y, lo.z)), min(hi.x, min(hi.y, hi.z)));
}
// Hierarchical empty-space traversal. Never jump over a finer cascade when entering it.
bool vxCascadeHit(vec3 start, vec3 direction, float limit, out float distance, out int hitCascade,
        out ivec3 hitCell, out bool exhausted)
{
    vec3 ray = mix(direction, vec3(1e-7), lessThan(abs(direction), vec3(1e-7)));
    distance = 0.0;
    exhausted = false;
    vec2 outer = vxCascadeIntersection(start, ray, vxCascadeBounds(vxCascadeCount() - 1));
    distance = max(0.0, outer.x + 0.0001);
    float end = min(limit, outer.y);
    for (int iteration = 0; iteration < 512 && distance < end; ++iteration)
    {
        vec3 p = start + ray * distance;
        int c = vxCascadeAt(p, 0.0);
        if (c < 0) return false;
        vec4 b = vxCascadeBounds(c);
        uvec4 grid = vxCascadeGrid(c);
        int n = int(grid.x), level = min(6, findMSB(n));
        float cell = b.w / float(n);
        vec3 local = (p - b.xyz) / cell;
        ivec3 block;
        for (; level >= 0; --level)
        {
            block = ivec3(floor(local / float(1 << level)));
            int address = int(grid.y) + vxCascadeMipOffset(n, level) + vxCascadeIndex(block, n >> level);
            if (cascadeWords[address] == 0u) break;
        }
        if (level < 0)
        {
            hitCascade = c;
            hitCell = block;
            return true;
        }
        float width = cell * float(1 << level);
        vec3 edge = b.xyz + (vec3(block) + step(vec3(0), ray)) * width;
        vec3 times = (edge - p) / ray;
        float stepSize = min(times.x, min(times.y, times.z));
        stepSize = min(stepSize, vxCascadeIntersection(p, ray, b).y);
        for (int fine = 0; fine < c; ++fine)
        {
            vec2 interval = vxCascadeIntersection(p, ray, vxCascadeBounds(fine));
            if (interval.x > 0.0 && interval.x < interval.y) stepSize = min(stepSize, interval.x);
        }
        distance += max(0.0, stepSize) + cell * 0.002;
    }
    exhausted = distance < end;
    return false;
}
vec4 vxCascadeTrace(vec3 position, vec3 normal, vec3 direction, float slope)
{
    int c = vxCascadeAt(position, 0.0);
    if (c < 0) return vec4(0);
    float cell = vxCascadeCell(c);
    vec3 start = position + normal * cell + direction * (0.25 * cell);
    float distance;
    int hitCascade;
    ivec3 hitCell;
    bool exhausted;
    bool hit = vxCascadeHit(start, direction, 2.0 * vxCascadeBounds(vxCascadeCount() - 1).w,
            distance, hitCascade, hitCell, exhausted);
    vec4 sum = vec4(0);
    float t = cell;
    for (int i = 0; i < 64 && t < distance && sum.a < 0.99; ++i)
    {
        float diameter = max(cell, slope * t);
        if (slope > 0.0 && diameter > cell)
        {
            vec4 value = vxCascadeSampleBlended(start + direction * t, direction, diameter);
            sum += (1.0 - sum.a) * value;
        }
        t += max(cell, diameter);
    }
    if (hit)
    {
        uvec4 grid = vxCascadeGrid(hitCascade);
        vec3 colour = vxCascadeFlux(int(grid.z) + 2 * vxCascadeIndex(hitCell, int(grid.x))).rgb;
        sum += (1.0 - sum.a) * vec4(colour, 1);
    }
    else if (exhausted) sum.a = 1.0;
    return sum;
}
bool vxCascadeInside(vec3 p)
{
    return vxCascadeContains(vxCascadeCount() - 1, voxelOcclusionMin + p * voxelOcclusionExtent);
}
vec4 vxCascadeReflection(vec3 p, vec3 normal, vec3 direction, float roughness)
{
    return vxCascadeTrace(voxelOcclusionMin + p * voxelOcclusionExtent, normal, direction, 2.0 * roughness * roughness);
}
float vxCascadeVisibility(vec3 position, vec3 normal, vec3 direction, float limit)
{
    int c = vxCascadeAt(position, 0.0);
    if (c < 0) return 1.0;
    float cell = vxCascadeCell(c), distance;
    int hitCascade;
    ivec3 hitCell;
    bool exhausted;
    bool hit = vxCascadeHit(position + cell * (0.8 * normal + 0.55 * direction), direction,
            max(0.0, limit - 1.5 * cell), distance, hitCascade, hitCell, exhausted);
    return hit || exhausted ? 0.0 : 1.0;
}
float vxCascadeAmbientVisibility(vec3 position, vec3 direction, float radius)
{
    int c = vxCascadeAt(position, 0.0);
    if (c < 0) return 1.0;
    float width = vxCascadeCell(c);
    vec3 origin = (position - vxCascadeBounds(c).xyz) / width;
    vec3 edge = mix(fract(origin), 1.0 - fract(origin), greaterThan(direction, vec3(0)));
    vec3 times = mix(vec3(1e10), edge / max(abs(direction), vec3(1e-8)), greaterThan(abs(direction), vec3(1e-8)));
    float start = (min(times.x, min(times.y, times.z)) + 0.002) * width;
    if (start >= radius) return 1.0;
    float distance;
    int hitCascade;
    ivec3 hitCell;
    bool exhausted;
    if (!vxCascadeHit(position + direction * start, direction, radius - start, distance, hitCascade, hitCell, exhausted))
    return exhausted ? 0.0 : 1.0;
    return smoothstep(0.0, radius, start + distance);
}
#ifdef CASCADE_ENVIRONMENT
vec3 vxCascadeCone(vec3 p, vec3 n, vec3 ray)
{
    vec4 value = vxCascadeTrace(p, n, ray, 0.9);
    float lod = log2(max(1.0, 0.45 * float(textureSize(coneEnvironment, 0).x)));
    return value.rgb + float(useEnvironment) * (1.0 - value.a) * 3.14159265 * textureLod(coneEnvironment, ray, lod).rgb;
}
vec3 vxCascadeHemisphere(vec3 origin, vec3 normal, float unusedCell)
{
    vec3 p = voxelOcclusionMin + origin * voxelOcclusionExtent;
    vec3 tangent = normalize(cross(abs(normal.y) < 0.99 ? vec3(0, 1, 0) : vec3(1, 0, 0), normal));
    vec3 bitangent = cross(normal, tangent);
    return 0.28 * vxCascadeCone(p, normal, normal) +
    0.18 * vxCascadeCone(p, normal, normalize(normal + tangent)) +
    0.18 * vxCascadeCone(p, normal, normalize(normal - tangent)) +
    0.18 * vxCascadeCone(p, normal, normalize(normal + bitangent)) +
    0.18 * vxCascadeCone(p, normal, normalize(normal - bitangent));
}
#define traceHemisphere vxCascadeHemisphere
#endif
#define inside vxCascadeInside
#define traceReflection vxCascadeReflection
#define voxelVisibility vxCascadeVisibility
#define voxelAmbientVisibility vxCascadeAmbientVisibility
