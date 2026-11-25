#include "opengl/voxel/voxel_visibility.glsl"

layout(binding = 9) uniform sampler3D directionalRadiance[6];
uniform bool directionalMips;

vec4 sampleConeRadiance(vec3 p, vec3 direction, float lod)
{
    if (!directionalMips) return textureLod(radianceVolume, p, lod);
    if (lod <= 0.0) return textureLod(radianceVolume, p, 0.0);

    float directionalLod = max(0.0, lod - 1.0);
    vec4 x = direction.x >= 0.0 ? textureLod(directionalRadiance[0], p, directionalLod)
                                 : textureLod(directionalRadiance[1], p, directionalLod);
    vec4 y = direction.y >= 0.0 ? textureLod(directionalRadiance[2], p, directionalLod)
                                 : textureLod(directionalRadiance[3], p, directionalLod);
    vec4 z = direction.z >= 0.0 ? textureLod(directionalRadiance[4], p, directionalLod)
                                 : textureLod(directionalRadiance[5], p, directionalLod);
    vec3 weight = direction * direction;
    vec4 radiance = x * weight.x + y * weight.y + z * weight.z;
    if (lod < 1.0) return mix(textureLod(radianceVolume, p, 0.0), radiance, lod);
    return radiance;
}

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
    ivec3 size = textureSize(radianceVolume, 0);
    vec3 radiance = vec3(0);
    float total = 0.0;
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 2; ++x)
        {
            ivec3 tap = base;
            tap[u] += x;
            tap[v] += y;
            if (any(lessThan(tap, ivec3(0))) || any(greaterThanEqual(tap, size))) continue;
            vec4 sampleValue = texelFetch(radianceVolume, tap, 0);
            vec2 weight = mix(1.0 - fraction, fraction, vec2(x, y));
            float w = weight.x * weight.y * sampleValue.a;
            radiance += sampleValue.rgb * w;
            total += w;
        }
    return total > 0.0 ? radiance / total : texelFetch(radianceVolume, cell, 0).rgb;
}

vec3 traceCone(vec3 origin, vec3 direction, float cellSize)
{
    vec4 sum = vec4(0);
    float distance = cellSize;
    float hitDistance;
    ivec3 hitCell;
    bool blocked = voxelFirstHit(origin / cellSize, direction, 0.4 / cellSize, hitDistance, hitCell);
    float stop = blocked ? hitDistance * cellSize : 0.4;
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
