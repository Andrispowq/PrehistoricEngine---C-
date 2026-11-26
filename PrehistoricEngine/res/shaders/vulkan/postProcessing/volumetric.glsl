layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColour;
layout(set = 0, binding = 0) uniform sampler2D scene;
layout(set = 0, binding = 1) uniform sampler2D bloom;
#if MULTISAMPLED
layout(set = 1, binding = 0) uniform sampler2DMS sceneDepth;
#else
layout(set = 1, binding = 0) uniform sampler2D sceneDepth;
#endif
layout(set = 1, binding = 2) uniform samplerCube irradianceMap;
const int MaxSpheres = 8;
layout(std140, set = 1, binding = 1) uniform Volumes
{
    mat4 inverseViewProjection;
    vec4 cameraAndCount;
    vec4 sphereBounds[MaxSpheres];
    vec4 sphereMedia[MaxSpheres];
    vec4 anisotropies[2];
    vec4 voxelBounds;
    ivec4 voxelOptions;
};
#define sphereCount int(cameraAndCount.w)
float volumeAnisotropy(int i) { return anisotropies[i / 4][i % 4]; }
#define SHADOW_SET 1
#include "../shadow/cascades.glsl"
layout(std430, set = 1, binding = 9) readonly buffer Grid { uint cells[]; };
int voxelGridSize() { return voxelOptions.x; }
#define voxelOcclusionExtent voxelBounds.w
#define voxelOcclusionMin voxelBounds.xyz
#include "../voxel/occlusion.glsl"
float directionalVisibility(vec3 p, vec3 n, float bias, bool volume)
{
    if (voxelOptions.x > 0) return voxelVisibility(p, n, sunDirection.xyz, 2.0 * voxelBounds.w);
    return cascadeSunVisibility(p, n, bias, volume);
}
#include "../../shared/postProcessing/volumetric.glsl"
layout(push_constant) uniform Settings { vec4 options; };

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(sceneDepth, pixel, 0).r;
    #if MULTISAMPLED
    // Stop at the nearest covered surface along silhouette edges.
    for (int i = 1; i < int(options.y); ++i)
    depth = min(depth, texelFetch(sceneDepth, pixel, i).r);
    #endif
    vec2 ndc = uv * 2.0 - 1.0;
    vec3 origin = unprojectVolumeRay(ndc, 0.0);
    vec3 direction = normalize(origin - cameraAndCount.xyz);
    float distanceToSurface = max(0.0, dot(unprojectVolumeRay(ndc, depth) - origin, direction));
    vec4 background = texelFetch(scene, pixel, 0);
    if (options.x > 0.0) background.rgb += texture(bloom, uv).rgb;
    float offset = volumeSampleOffset(pixel);
    outColour = vec4(compositeVolumes(background.rgb, origin, direction, distanceToSurface, offset), background.a);
}
