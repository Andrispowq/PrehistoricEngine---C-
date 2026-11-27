// Water receives radiance; it never contributes occupancy or triggers a voxel bake.
vec3 waterVoxelReflection(vec3 position, vec3 normal, vec3 eye, vec3 captured)
{
    if (waterVoxelOptions.z == 0 || waterVoxelOptions.x <= 0 || waterVoxelOptions.y <= 0) return captured;
    vec3 origin = (position - waterVoxelBounds.xyz) / waterVoxelBounds.w;
    if (!inside(origin)) return captured;
    vec4 hit = traceReflection(origin, normal, reflect(-eye, normal), 0.08);
    return captured + clamp(waterVoxelOptions.y, 0.0, 1.0) * (hit.rgb - hit.a * captured);
}
