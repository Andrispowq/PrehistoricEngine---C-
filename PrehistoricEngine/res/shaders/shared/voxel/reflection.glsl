// Premultiplied scene radiance and coverage. The uncovered fraction sees the prefiltered sky.
vec4 traceReflection(vec3 origin, vec3 normal, vec3 direction, float roughness)
{
    float size = float(voxelGridSize());
    vec3 start = origin * size + normal + 0.25 * direction;
    if (!inside(start / size)) return vec4(0);

    float distance;
    ivec3 cell;
    bool hit = voxelFirstHit(start, direction, 1.733 * size, distance, cell);
    float slope = 2.0 * roughness * roughness;
    vec4 sum = vec4(0);
    // The DDA resolves the sub-voxel-width portion, including the entire mirror ray.
    if (slope * distance > 1.0)
    {
        float t = 1.0 / slope;
        for (int stepIndex = 0; stepIndex < 24 && t < distance && sum.a < 0.99; ++stepIndex)
        {
            float diameter = t * slope;
            vec4 sampleValue = sampleConeRadiance((start + direction * t) / size, direction, log2(diameter));
            sum += (1.0 - sum.a) * sampleValue;
            t += diameter;
        }
    }
    if (hit)
    {
        vec3 colour = filterHitRadiance(cell, start, direction, distance);
        float lod = log2(max(1.0, distance * slope));
        if (lod > 0.0)
        {
            vec4 footprint = sampleConeRadiance((start + direction * distance) / size, direction, lod);
            if (footprint.a > 0.001)
            colour = mix(colour, footprint.rgb / footprint.a, min(lod, 1.0));
        }
        sum += (1.0 - sum.a) * vec4(colour, 1);
    }
    else if (inside((start + direction * distance) / size))
    sum.a = 1.0; // Traversal budget exhausted before reaching the sky.
    return sum;
}

float voxelReflectionWeight(float metalness, float roughness, float strength)
{
    // Fully rough dielectrics keep their broad environment lobe.
    return clamp(strength, 0.0, 1.0) * mix(1.0 - roughness, 1.0, metalness);
}

vec3 reflectionCorrection(vec4 traced, vec3 sky, vec3 brdf, float weight)
{
    return weight * brdf * (traced.rgb - traced.a * sky);
}
