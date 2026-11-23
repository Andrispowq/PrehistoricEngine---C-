#version 450
#extension GL_GOOGLE_include_directive : require
#include "../pbr/scene.glsl"
#include "params.glsl"
#include "../../shared/terrain/lighting.glsl"
layout(location=2) in vec3 basePosition;
#include "../../shared/terrain/detail.glsl"
layout(set=1, binding=1) uniform sampler2D albedo0;
layout(set=1, binding=2) uniform sampler2D normal0;
layout(set=1, binding=3) uniform sampler2D mrot0;
layout(set=1, binding=4) uniform sampler2D albedo1;
layout(set=1, binding=5) uniform sampler2D normal1;
layout(set=1, binding=6) uniform sampler2D mrot1;
layout(set=1, binding=7) uniform sampler2D albedo2;
layout(set=1, binding=8) uniform sampler2D normal2;
layout(set=1, binding=9) uniform sampler2D mrot2;

layout(set=1, binding=10) uniform texture2D terrainNormal;
layout(set=1, binding=11) uniform texture2D terrainSplat;
layout(set=1, binding=15) uniform sampler mapSampler;
layout(set=1, binding=12) uniform sampler2D emission0;
layout(set=1, binding=13) uniform sampler2D emission1;
layout(set=1, binding=14) uniform sampler2D emission2;

void accumulateLayer(float weight, int i, sampler2D albedoTex, sampler2D mrotTex,
                     inout vec3 colour, inout vec4 mrot)
{
	vec2 uv = vec2(texture_FS.x, -texture_FS.y) * tiling[i];
	colour += weight * pow(texture(albedoTex, uv).rgb, vec3(2.2));
	mrot += weight * mix(layerMrot[i], texture(mrotTex, uv), equal(layerMrot[i], vec4(-1)));
}

void main()
{
	vec3 N = texture(sampler2D(terrainNormal, mapSampler), texture_FS).rbg * 2.0 - 1.0;
	vec3 weights = texture(sampler2D(terrainSplat, mapSampler), texture_FS).rgb;
	vec3 colour = vec3(0);
	vec4 mrot = vec4(0);
	accumulateLayer(weights.x, 0, albedo0, mrot0, colour, mrot);
	accumulateLayer(weights.y, 1, albedo1, mrot1, colour, mrot);
	accumulateLayer(weights.z, 2, albedo2, mrot2, colour, mrot);
	// Location 2 carries the interpolated surface position before fine displacement.
	vec3 derivativeTangent = terrainTangent(dFdx(basePosition), dFdy(basePosition), dFdx(texture_FS), dFdy(texture_FS));
	vec2 uv = texture_FS * vec2(1, -1);
	float dist = distance(cameraPosition, position_FS);
	if (dist < heightScales.w - 50.0)
	{
		vec3 T = emissionEnabled.w > 0.5 ? tangent_FS : derivativeTangent;
		vec3 bump = weights.x * (2.0 * texture(normal0, uv * tiling.x).rbg - 1.0)
			+ weights.y * (2.0 * texture(normal1, uv * tiling.y).rbg - 1.0)
			+ weights.z * (2.0 * texture(normal2, uv * tiling.z).rbg - 1.0);
		N = terrainDetailNormal(N, T, bump, dist, heightScales.w);
	}
	vec3 emission = weights.x * emissionEnabled.x * pow(texture(emission0, uv * tiling.x).rgb, vec3(2.2))
		+ weights.y * emissionEnabled.y * pow(texture(emission1, uv * tiling.y).rgb, vec3(2.2))
		+ weights.z * emissionEnabled.z * pow(texture(emission2, uv * tiling.z).rgb, vec3(2.2));
	vec3 V = normalize(cameraPosition - position_FS);
	vec3 F0 = mix(vec3(0.04), colour, mrot.r);
	vec3 lit = terrainAmbient(N, V, F0, colour, mrot.r, mrot.g, mrot.b,
		irradianceMap, prefilterMap, brdfLUT, maxReflectionLOD);
	for (int i = 0; i < numberOfLights; ++i)
		lit += terrainLight(N, V, F0, colour, mrot.r, mrot.g,
			lights.position[i].xyz - position_FS, lights.colour[i].rgb, lights.intensity[i].rg);
	lit += emission * emissionFactor;
	outColour = vec4(isnan(lit.r) ? vec3(0) : lit, 1);
	outBloom = vec4(emission * emissionFactor, 0);
}
