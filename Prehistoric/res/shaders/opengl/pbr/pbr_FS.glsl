#version 330

layout (location = 0) out vec4 positionMetallic;
layout (location = 1) out vec4 albedoRoughness;
layout (location = 2) out vec4 normalLit;
layout (location = 3) out vec4 emissionExtra;

in vec3 position_FS;
in vec2 texture_FS;
in vec3 normal_FS;
in vec3 tangent_FS;

struct Material
{
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D displacementMap;
	sampler2D metallicMap;
	sampler2D roughnessMap;
	sampler2D occlusionMap;
	sampler2D emissionMap;
	
	vec3 colour;
	int usesNormalMap;
	float heightScale;
	float metallic;
	float roughness;
	float ambientOcclusion;
	vec3 emission;
};

uniform Material material;
uniform vec3 cameraPosition;
uniform int highDetailRange;

vec3 getColour(sampler2D map, vec3 alternateValue, vec2 texCoords, bool power)
{
	if (alternateValue.r == -1)
		if (power)
			return pow(texture(map, texCoords).rgb, vec3(2.2));
		else
			return texture(map, texCoords).rgb;
	else
		return alternateValue;
}

vec3 getColour(sampler2D map, float alternateValue, vec2 texCoords)
{
	if(alternateValue == -1)
		return texture(map, texCoords).rgb;
	else
		return vec3(alternateValue);
}

void main()
{
	vec3 albedoColour = getColour(material.albedoMap, material.colour, texture_FS, true);
	
	float metallic = getColour(material.metallicMap, material.metallic, texture_FS).r;
	float roughness = getColour(material.roughnessMap, material.roughness, texture_FS).r;
	float occlusion = getColour(material.occlusionMap, material.ambientOcclusion, texture_FS).r;
	
	vec3 emission = getColour(material.emissionMap, material.emission, texture_FS, false);

	float dist = length(cameraPosition - position_FS);
	vec3 normal = normalize(normal_FS);	
	if(dist < highDetailRange && material.usesNormalMap == 1)
	{
		float attenuation = clamp(-dist / highDetailRange + 1.0, 0.0, 1.0);

		normal = 2.0 * texture(material.normalMap, texture_FS).rbg - 1.0;
		normal = normalize(normal);
		normal.xz *= attenuation;

		vec3 B = normalize(cross(tangent_FS, normal_FS));
		mat3 tbn = mat3(tangent_FS, normal_FS, B);

		normal = tbn * normal;
	}
	
	vec3 N = normalize(normal);

	positionMetallic = vec4(position_FS, metallic);
	albedoRoughness = vec4(albedoColour, roughness);
	normalLit = vec4(N, 1.0);
	emissionExtra = vec4(emission, occlusion);
}