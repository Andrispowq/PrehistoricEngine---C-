bool retainedVoxel(ivec3 cell, ivec3 delta, int size)
{
    ivec3 old = cell + delta;
    return all(greaterThanEqual(old, ivec3(0))) && all(lessThan(old, ivec3(size)));
}

bool retainedTriangle(vec3 a, vec3 b, vec3 c, ivec3 delta, int size)
{
    vec3 lo = min(a, min(b, c)) + vec3(delta);
    vec3 hi = max(a, max(b, c)) + vec3(delta);
    return all(greaterThanEqual(lo, vec3(0))) && all(lessThan(hi, vec3(size)));
}
