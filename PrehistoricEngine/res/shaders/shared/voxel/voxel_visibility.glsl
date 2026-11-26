bool voxelFirstHit(vec3 start, vec3 direction, float limit, out float t, out ivec3 hitCell)
{
    float size = float(voxelGridSize());
    vec3 ray = mix(direction, vec3(1e-6), lessThan(abs(direction), vec3(1e-6)));
    int topLevel = min(6, voxelTopLevel());
    int level = topLevel;
    t = 0.0;
    for (int iteration = 0; iteration < 192 && t < limit; ++iteration)
    {
        vec3 p = start + ray * t;
        if (any(lessThan(p, vec3(0))) || any(greaterThanEqual(p, vec3(size)))) break;
        float width = float(1 << level);
        ivec3 block = ivec3(floor(p / width));
        if (occupiedBlock(block, level))
        {
            if (level == 0)
            {
                hitCell = block;
                return true;
            }
            --level;
            continue;
        }

        vec3 edge = (vec3(block) + step(vec3(0), ray)) * width;
        vec3 exit = (edge - p) / ray;
        t += max(0.0, min(exit.x, min(exit.y, exit.z))) + 0.002;
        level = min(level + 1, topLevel);
    }
    return false;
}

float voxelVisibility(vec3 position, vec3 normal, vec3 direction, float maxDistance)
{
    float size = float(voxelGridSize());
    float voxelSize = voxelOcclusionExtent / size;
    vec3 start = (position - voxelOcclusionMin) / voxelSize;
    if (any(lessThan(start, vec3(0))) || any(greaterThanEqual(start, vec3(size)))) return 1.0;

    // Move off the receiver and stop short of a local light's endpoint.
    start += normal * 0.8 + direction * 0.55;
    float limit = maxDistance / voxelSize - 1.5;
    float hitDistance;
    ivec3 hitCell;
    return voxelFirstHit(start, direction, limit, hitDistance, hitCell) ? 0.0 : 1.0;
}

// AO integrates only the requested world-space segment. Occupancy says nothing about
// the surface's position inside its own cell, so begin at that cell's first exit.
float voxelAmbientVisibility(vec3 position, vec3 direction, float radius)
{
    float cellWidth = voxelOcclusionExtent / float(voxelGridSize());
    vec3 origin = (position - voxelOcclusionMin) / cellWidth;
    if (any(lessThan(origin, vec3(0))) || any(greaterThanEqual(origin, vec3(voxelGridSize())))) return 1.0;
    vec3 edge = mix(fract(origin), 1.0 - fract(origin), greaterThan(direction, vec3(0)));
    vec3 times = mix(vec3(1e10), edge / max(abs(direction), vec3(1e-8)), greaterThan(abs(direction), vec3(1e-8)));
    float start = min(times.x, min(times.y, times.z)) + 0.002;
    float limit = radius / cellWidth;
    if (start >= limit) return 1.0;

    ivec3 hitCell;
    float distance;
    if (!voxelFirstHit(origin + direction * start, direction, limit - start, distance, hitCell)) return 1.0;
    return smoothstep(0.0, limit, start + distance);
}
