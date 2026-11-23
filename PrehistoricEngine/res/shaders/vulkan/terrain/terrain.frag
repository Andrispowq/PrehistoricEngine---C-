#version 450
#extension GL_GOOGLE_include_directive : require
#include "../pbr/scene.glsl"
#include "params.glsl"
#include "../../shared/terrain/heightNormal.glsl"
layout(set=1, binding=0) uniform sampler2D heightMap;
#include "../../shared/terrain/layers.glsl"
layout(set=1, binding=1) uniform sampler2D albedo0;
layout(set=1, binding=2) uniform sampler2D normal0;
layout(set=1, binding=3) uniform sampler2D mrot0;
layout(set=1, binding=4) uniform sampler2D albedo1;
layout(set=1, binding=5) uniform sampler2D normal1;
layout(set=1, binding=6) uniform sampler2D mrot1;
layout(set=1, binding=7) uniform sampler2D albedo2;
layout(set=1, binding=8) uniform sampler2D normal2;
layout(set=1, binding=9) uniform sampler2D mrot2;

void accumulateLayer(float weight, int i, sampler2D albedoTex, sampler2D normalTex, sampler2D mrotTex,
                     inout vec3 colour, inout vec3 normal, inout vec4 mrot)
{
	vec2 uv = vec2(texture_FS.x, -texture_FS.y) * tiling[i];
	colour += weight * pow(texture(albedoTex, uv).rgb, vec3(2.2));
	normal += weight * (2.0 * texture(normalTex, uv).xyz - 1.0);
	mrot += weight * mix(layerMrot[i], texture(mrotTex, uv), lessThan(layerMrot[i], vec4(0)));
}

void main()
{
	vec3 N = normalize(normal_FS);
	vec2 texel = vec2(1, -1) / vec2(textureSize(heightMap, 0));
	vec3 slopeNormal = terrainHeightNormal(heightMap, vec2(texture_FS.x, 1.0 - texture_FS.y), texel, tessellation.w);
	vec3 weights = terrainLayerWeights(slopeNormal.z);
	vec3 colour = vec3(0), normal = vec3(0);
	vec4 mrot = vec4(0);
	accumulateLayer(weights.x, 0, albedo0, normal0, mrot0, colour, normal, mrot);
	accumulateLayer(weights.y, 1, albedo1, normal1, mrot1, colour, normal, mrot);
	accumulateLayer(weights.z, 2, albedo2, normal2, mrot2, colour, normal, mrot);
	vec3 T = normalize(tangent_FS - N * dot(tangent_FS, N));
	N = normalize(mat3(T, cross(T, N), N) * normal);
	shadeSurface(colour, mrot.r, max(mrot.g, 0.04), mrot.b, N, 0.0);
}
