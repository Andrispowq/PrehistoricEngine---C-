// Logarithmic RGB8 packing covers HDR emitters through 1023 per channel.
vec3 encodeVoxelRadiance(vec3 radiance)
{
    return clamp(log2(1.0 + radiance) / 10.0, 0.0, 1.0);
}

vec3 decodeVoxelRadiance(uint encodedRadiance)
{
    return exp2(unpackUnorm4x8(encodedRadiance).rgb * 10.0) - 1.0;
}

vec2 octFold(vec2 p)
{
    return (1.0 - abs(p.yx)) * mix(vec2(-1), vec2(1), greaterThanEqual(p, vec2(0)));
}

uint packBounceSurface(vec3 reflectance, vec3 normal)
{
    normal /= dot(abs(normal), vec3(1));
    vec2 oct = normal.z >= 0.0 ? normal.xy : octFold(normal.xy);
    uvec2 direction = uvec2(round((oct * 0.5 + 0.5) * 255.0));
    uvec3 colour = uvec3(round(clamp(reflectance, 0.0, 0.95) * 31.0));
    return 0x80000000u | (colour.r << 26) | (colour.g << 21) | (colour.b << 16)
    | (direction.x << 8) | direction.y;
}

vec3 unpackBounceReflectance(uint surface)
{
    return vec3((surface >> 26) & 31u, (surface >> 21) & 31u, (surface >> 16) & 31u) / 31.0;
}

vec3 unpackBounceNormal(uint surface)
{
    vec2 oct = vec2((surface >> 8) & 255u, surface & 255u) * (2.0 / 255.0) - 1.0;
    vec3 normal = vec3(oct, 1.0 - abs(oct.x) - abs(oct.y));
    if (normal.z < 0.0) normal.xy = octFold(normal.xy);
    return normalize(normal);
}
