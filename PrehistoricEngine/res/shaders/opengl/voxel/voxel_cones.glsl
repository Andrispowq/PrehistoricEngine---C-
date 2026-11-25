bool inside(vec3 p)
{
    return all(greaterThanEqual(p, vec3(0))) && all(lessThan(p, vec3(1)));
}

vec3 traceCone(vec3 origin, vec3 direction, float cellSize)
{
    vec4 sum = vec4(0);
    float distance = cellSize;
    for (int i = 0; i < 12 && sum.a < 0.95 && distance < 0.4; ++i)
    {
        vec3 p = origin + direction * distance;
        if (!inside(p)) break;
        float diameter = max(cellSize, distance * 0.9);
        vec4 sampleValue = textureLod(radianceVolume, p, log2(diameter / cellSize));
        sum += (1.0 - sum.a) * sampleValue;
        distance += diameter;
    }
    return sum.rgb;
}

vec3 traceHemisphere(vec3 origin, vec3 normal, float cellSize)
{
    vec3 tangent = normalize(cross(abs(normal.y) < 0.99 ? vec3(0, 1, 0) : vec3(1, 0, 0), normal));
    vec3 bitangent = cross(normal, tangent);
    origin += normal * cellSize * 1.25;
    vec3 radiance = traceCone(origin, normal, cellSize) * 0.28;
    radiance += traceCone(origin, normalize(normal + tangent), cellSize) * 0.18;
    radiance += traceCone(origin, normalize(normal - tangent), cellSize) * 0.18;
    radiance += traceCone(origin, normalize(normal + bitangent), cellSize) * 0.18;
    radiance += traceCone(origin, normalize(normal - bitangent), cellSize) * 0.18;

    return radiance;
}
