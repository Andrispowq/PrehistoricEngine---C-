bool triangleAxis(vec3 axis, vec3 a, vec3 b, vec3 c)
{
    vec3 projection = vec3(dot(axis, a), dot(axis, b), dot(axis, c));
    float radius = 0.50001 * dot(abs(axis), vec3(1));
    return min(projection.x, min(projection.y, projection.z)) <= radius &&
    max(projection.x, max(projection.y, projection.z)) >= -radius;
}

bool terrainTriangleOverlaps(vec3 a, vec3 b, vec3 c, ivec3 cell)
{
    vec3 center = vec3(cell) + 0.5;
    a -= center; b -= center; c -= center;
    vec3 edges[3] = vec3[](b - a, c - b, a - c);
    if (!triangleAxis(cross(edges[0], edges[1]), a, b, c)) return false;
    for (int i = 0; i < 3; ++i)
    {
        if (!triangleAxis(cross(edges[i], vec3(1, 0, 0)), a, b, c) ||
            !triangleAxis(cross(edges[i], vec3(0, 1, 0)), a, b, c) ||
            !triangleAxis(cross(edges[i], vec3(0, 0, 1)), a, b, c)) return false;
    }
    return true; // Axis-aligned bounds are checked by the caller.
}

vec3 terrainVoxelPosition(vec2 uv)
{
    float h = terrainHeight(uv);
    vec3 p = (terrainModel * vec4(uv.x, h, uv.y, 1)).xyz;
    p.y = h * terrainHeights.w;
    if (terrainEye.w > 0)
    {
        float dist = distance(p, terrainEye.xyz);
        p.y += terrainDisplacement(terrainDetail(uv), terrainHeights.xyz,
                terrainWeights(uv), dist, terrainEye.w);
    }
    return p;
}

void terrainTriangle(vec3 a, vec3 b, vec3 c, vec2 ua, vec2 ub, vec2 uc)
{
    int size = int(terrainOptions.z);
    float scale = float(size) / terrainGrid.w;
    a = (a - terrainGrid.xyz) * scale;
    b = (b - terrainGrid.xyz) * scale;
    c = (c - terrainGrid.xyz) * scale;
    vec3 lower = min(a, min(b, c)), upper = max(a, max(b, c));
    if (any(lessThan(upper, vec3(0))) || any(greaterThanEqual(lower, vec3(size)))) return;
    ivec3 lo = max(ivec3(floor(lower - 0.00001)), ivec3(0));
    ivec3 hi = min(ivec3(floor(upper + 0.00001)), ivec3(size - 1));
    vec3 e = b - a, f = c - a;
    float ee = dot(e, e), ef = dot(e, f), ff = dot(f, f);
    float det = ee * ff - ef * ef;
    if (det < 1e-12) return;
    for (int z = lo.z; z <= hi.z; ++z)
    for (int y = lo.y; y <= hi.y; ++y)
    for (int x = lo.x; x <= hi.x; ++x)
    {
        ivec3 cell = ivec3(x, y, z);
        if (!terrainTriangleOverlaps(a, b, c, cell)) continue;
        vec3 at = vec3(cell) + 0.5 - a;
        vec2 bary = max(vec2(ff * dot(at, e) - ef * dot(at, f),
                ee * dot(at, f) - ef * dot(at, e)) / det, vec2(0));
        bary /= max(1.0, bary.x + bary.y);
        vec2 uv = ua + bary.x * (ub - ua) + bary.y * (uc - ua);
        vec3 base, emission;
        float metallic;
        terrainMaterial(uv, base, metallic, emission);
        storeTerrainVoxel(cell, terrainNormal(uv), base, metallic, emission);
    }
}

void main()
{
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(id, ivec2(terrainRegion.zw)))) return;
    vec2 stepUV = 1.0 / terrainOptions.xy;
    vec2 uv = (vec2(id) + terrainRegion.xy) * stepUV;
    vec2 u[4] = vec2[](uv, uv + vec2(stepUV.x, 0), uv + stepUV, uv + vec2(0, stepUV.y));
    vec3 p[4];
    for (int i = 0; i < 4; ++i) p[i] = terrainVoxelPosition(u[i]);
    terrainTriangle(p[0], p[2], p[1], u[0], u[2], u[1]);
    terrainTriangle(p[0], p[3], p[2], u[0], u[3], u[2]);
}
