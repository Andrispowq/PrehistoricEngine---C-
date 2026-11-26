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

vec3 baseRadiance(ivec3 p) { return texelFetch(radianceVolume, p, 0).rgb; }
vec4 baseSample(ivec3 p) { return texelFetch(radianceVolume, p, 0); }
#include "shared/voxel/cone_hit.glsl"
