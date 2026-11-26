#version 430

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outPositionMetallic;
layout(location = 2) out vec4 outAlbedoRoughness;
layout(location = 3) out vec4 outNormal;
layout(location = 4) out vec4 outBloom;

struct Material
{
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D mrotMap;
	sampler2D emissionMap;

	float heightScale;
	float horizontalScale;
	vec4 mrot;
};

struct PointLight
{
	vec4 position;
	vec4 colour;
	vec4 intensity_radius;
};

struct VisibleIndex
{
	int index;
};

layout(std430, binding = 0) readonly buffer LightBuffer
{
	PointLight data[];
} lightBuffer;

layout(std430, binding = 1) readonly buffer VisibleLightIndicesBuffer
{
	VisibleIndex data[];
} visibleLightIndicesBuffer;

in vec3 position_FS;
in vec2 mapCoord_FS;
in vec3 tangent_FS;

uniform Material materials[3];
uniform vec3 emissionEnabled;

uniform sampler2D normalmap;
uniform sampler2D heightmap;
uniform sampler2D splatmap;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 cameraPosition;
uniform int highDetailRange;
uniform int numberOfTilesX;
uniform int captureLightCount;
uniform float max_reflection_lod;
uniform float threshold;

const float PI = 3.141592653589793;
const float emissionFactor = 3;

#include "shared/pbr/lighting.glsl"
#include "shared/terrain/lighting.glsl"
#include "shared/terrain/detail.glsl"

void main()
{
	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	uint index = uint(tileID.y * numberOfTilesX + tileID.x);

	float dist = length(cameraPosition - position_FS);
	float height = position_FS.y;

	vec3 N = texture(normalmap, mapCoord_FS).rbg * 2.0 - 1.0;	
	vec4 blendValues = texture(splatmap, mapCoord_FS);
	float[4] blendValueArray = float[](blendValues.r, blendValues.g, blendValues.b, blendValues.a);	
	if(dist < highDetailRange - 50)
	{
		vec3 bumpNormal = vec3(0);
		for(int i = 0; i < 3; i++)
		{
			bumpNormal += (2 * texture(materials[i].normalMap, mapCoord_FS * materials[i].horizontalScale).rbg - 1)
				* blendValueArray[i];
		}
		
		N = terrainDetailNormal(N, tangent_FS, bumpNormal, dist, highDetailRange);
	}
	
	vec3 albedoColour = vec3(0);
	vec3 emissionColour = vec3(0);
	
	float metallic;
	float roughness;
	float occlusion;
	
	vec4 mrot = vec4(0);
	for(int i = 0; i < 3; i++)
	{
		vec2 texCoords = mapCoord_FS * materials[i].horizontalScale;

		albedoColour += pow(texture(materials[i].albedoMap, texCoords).rgb, vec3(2.2)) * blendValueArray[i];
		emissionColour += emissionEnabled[i] * pow(texture(materials[i].emissionMap, texCoords).rgb, vec3(2.2)) * blendValueArray[i];

		vec4 mrot_i = materials[i].mrot;
		vec4 mrotMap = texture(materials[i].mrotMap, texCoords);
		if (mrot_i.r == -1)
		{
			mrot_i.r = mrotMap.r;
		}
		if (mrot_i.g == -1)
		{
			mrot_i.g = mrotMap.g;
		}
		if (mrot_i.b == -1)
		{
			mrot_i.b = mrotMap.b;
		}
		if (mrot_i.a == -1)
		{
			mrot_i.a = mrotMap.a;
		}

		mrot += mrot_i * blendValueArray[i];
	}

	metallic = mrot.r;
	roughness = mrot.g;
	occlusion = mrot.b;

    vec3 V = normalize(cameraPosition - position_FS);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedoColour, metallic);

    vec3 Lo = vec3(0);
	uint offset = index * 1024;
    for (uint i = 0; i < (captureLightCount >= 0 ? uint(captureLightCount) : 1024u); i++)
    {
		int lightIndex = captureLightCount >= 0 ? int(i) : visibleLightIndicesBuffer.data[offset + i].index;
        if (lightIndex == -1) break;
		PointLight light = lightBuffer.data[lightIndex];
		
		Lo += terrainLight(N, V, F0, albedoColour, metallic, roughness,
			light.position.xyz - position_FS, light.colour.rgb, light.intensity_radius.rg);
    }

    vec3 ambient = terrainAmbient(N, V, F0, albedoColour, metallic, roughness, occlusion,
        irradianceMap, prefilterMap, brdfLUT, max_reflection_lod);
    vec3 colour = ambient + Lo + emissionColour * emissionFactor;

	if (isnan(colour.r))
	{
		colour = vec3(0.0);
	}

	outColour = vec4(colour, 1);
	outPositionMetallic = vec4(position_FS, metallic);
	outAlbedoRoughness = vec4(albedoColour, roughness);
	outNormal = vec4(N, 1);
    outBloom = vec4(emissionColour * emissionFactor, 3.0 + occlusion);
}
