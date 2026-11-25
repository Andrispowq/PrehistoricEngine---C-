#include "occlusion.glsl"

int surfaceOffset() { return pyramidCount(); }
int emissionOffset() { return surfaceOffset() + volumeCount(); }
int radianceOffset() { return emissionOffset() + volumeCount(); }
int directionalOffset(int face)
{
    return radianceOffset() + 2 * (pyramidCount() + face * (pyramidCount() - volumeCount()));
}
int accumulatedOffset() { return directionalOffset(6); }
int nextOffset() { return accumulatedOffset() + 2 * volumeCount(); }
int fluxAddress(ivec3 p, int level, int face)
{
    int cell = cellAt(p, level);
    return face < 0 ? radianceOffset() + 2 * cell : directionalOffset(face) + 2 * (cell - volumeCount());
}
vec4 readFlux(int word) { return vec4(unpackHalf2x16(cells[word]), unpackHalf2x16(cells[word + 1])); }
vec4 baseSample(ivec3 p) { return readFlux(fluxAddress(p, 0, -1)); }
vec3 baseRadiance(ivec3 p) { return baseSample(p).rgb; }
vec4 sampleLevel(vec3 p, int level, int face)
{
    int width = voxelGridSize() >> level;
    vec3 at = clamp(p * float(width) - 0.5, vec3(0), vec3(width - 1));
    ivec3 base = ivec3(at);
    vec3 f = fract(at);
    vec4 value = vec4(0);
    for (int i = 0; i < 8; ++i)
    {
        ivec3 corner = ivec3(i & 1, (i >> 1) & 1, i >> 2);
        vec3 weights = mix(1.0 - f, f, vec3(corner));
        value += weights.x * weights.y * weights.z *
        readFlux(fluxAddress(min(base + corner, ivec3(width - 1)), level, face));
    }
    return value;
}
vec4 coneLevel(vec3 p, vec3 ray, int level)
{
    if (level == 0 || !directionalMips) return sampleLevel(p, level, -1);
    vec3 w = ray * ray;
    return w.x * sampleLevel(p, level, ray.x >= 0 ? 0 : 1) + w.y * sampleLevel(p, level, ray.y >= 0 ? 2 : 3) +
    w.z * sampleLevel(p, level, ray.z >= 0 ? 4 : 5);
}
vec4 sampleConeRadiance(vec3 p, vec3 ray, float lod)
{
    lod = clamp(lod, 0.0, float(voxelTopLevel()));
    int level = int(lod);
    vec4 low = coneLevel(p, ray, level);
    if (fract(lod) == 0.0) return low;
    return mix(low, coneLevel(p, ray, level + 1), fract(lod));
}
#include "../../shared/voxel/voxel_cones.glsl"
