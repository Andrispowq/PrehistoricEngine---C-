vec3 traceCone(vec3 origin, vec3 direction, float cellSize)
{
    vec4 sum = vec4(0);
    float distance = cellSize;
    float hitDistance;
    ivec3 hitCell;
    bool blocked = voxelFirstHit(origin / cellSize, direction, 1.733 / cellSize, hitDistance, hitCell);
    float stop = hitDistance * cellSize;
    // An exhausted traversal inside the grid is inconclusive. Only a grid exit sees sky.
    bool escaped = !blocked && !inside(origin + direction * stop);
    for (int i = 0; i < 12 && sum.a < 0.95 && distance < stop; ++i)
    {
        vec3 p = origin + direction * distance;
        if (!inside(p)) break;
        float diameter = max(cellSize, distance * 0.9);
        if (blocked) diameter = min(diameter, max(cellSize, 2.0 * (stop - distance)));
        vec4 sampleValue = sampleConeRadiance(p, direction, log2(diameter / cellSize));
        sum += (1.0 - sum.a) * sampleValue;
        distance += diameter;
    }
    if (blocked)
    sum.rgb += (1.0 - sum.a) * filterHitRadiance(hitCell, origin / cellSize, direction, hitDistance);
    else if (escaped && useEnvironment && sum.a < 1.0)
    {
        // The cone diameter grows by 0.9 per unit distance. A cube face spans two units.
        float skyLod = log2(max(1.0, 0.45 * float(textureSize(coneEnvironment, 0).x)));
        // Match the diffuse irradiance convention of ConvolveEnvironment for an open hemisphere.
        sum.rgb += (1.0 - sum.a) * 3.14159265 * textureLod(coneEnvironment, direction, skyLod).rgb;
    }
    return sum.rgb;
}

vec3 traceHemisphere(vec3 origin, vec3 normal, float cellSize)
{
    vec3 tangent = normalize(cross(abs(normal.y) < 0.99 ? vec3(0, 1, 0) : vec3(1, 0, 0), normal));
    vec3 bitangent = cross(normal, tangent);
    origin += normal * cellSize;
    vec3 radiance = traceCone(origin, normal, cellSize) * 0.28;
    radiance += traceCone(origin, normalize(normal + tangent), cellSize) * 0.18;
    radiance += traceCone(origin, normalize(normal - tangent), cellSize) * 0.18;
    radiance += traceCone(origin, normalize(normal + bitangent), cellSize) * 0.18;
    radiance += traceCone(origin, normalize(normal - bitangent), cellSize) * 0.18;

    return radiance;
}
