#version 450
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 indirect;
layout(set = 0, binding = 0) uniform sampler2D positions;
layout(set = 0, binding = 1) uniform sampler2D normals;
layout(set = 1, binding = 0, std430) readonly buffer Grid { uint cells[]; };
layout(push_constant) uniform Trace
{
    vec4 bounds;
    ivec4 options;
};
int voxelGridSize() { return options.x; }
#define directionalMips (options.y != 0)
#define voxelOcclusionMin bounds.xyz
#define voxelOcclusionExtent bounds.w
layout(set = 1, binding = 1) uniform samplerCube coneEnvironment;
const bool useEnvironment = true;
#include "grid.glsl"
#include "../../shared/voxel/voxel_cones.glsl"
#define CASCADE_ENVIRONMENT
#define cascadeWords cells
#define VOXEL_SKIP_ZERO_WEIGHT_SAMPLES
#include "../../shared/voxel/cascades.glsl"
void main()
{
    ivec2 p = min(ivec2(gl_FragCoord.xy) * options.z + options.z / 2, textureSize(normals, 0) - 1);
    vec4 n = texelFetch(normals, p, 0);
    vec3 pos = (texelFetch(positions, p, 0).xyz - bounds.xyz) / bounds.w;
    indirect = vec4(0);
    if (n.a >= 0.99 && dot(n.xyz, n.xyz) > 0.01 && inside(pos))
    indirect = vec4(traceHemisphere(pos, normalize(n.xyz), 1.0 / voxelGridSize()), 1);
}
