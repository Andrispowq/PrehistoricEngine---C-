vec3 CubeDirection(ivec3 pixel, int resolution)
{
    vec2 uv = vec2(pixel.xy) / float(resolution) * 2.0 - 1.0;
    // Cube layer order: +X, -X, +Y, -Y, +Z, -Z.
    if (pixel.z == 0) return normalize(vec3(1.0, -uv.y, -uv.x));
    if (pixel.z == 1) return normalize(vec3(-1.0, -uv.y, uv.x));
    if (pixel.z == 2) return normalize(vec3(uv.x, 1.0, uv.y));
    if (pixel.z == 3) return normalize(vec3(uv.x, -1.0, -uv.y));
    if (pixel.z == 4) return normalize(vec3(uv.x, -uv.y, 1.0));
    return normalize(vec3(-uv.x, -uv.y, -1.0));
}

ivec2 PanoramaTexel(vec3 direction, ivec2 extent)
{
    vec2 uv = vec2(atan(-direction.z, direction.x), asin(direction.y)) * vec2(0.1591, 0.3183) + 0.5;
    return clamp(ivec2(uv * vec2(extent)), ivec2(0), extent - 1);
}
