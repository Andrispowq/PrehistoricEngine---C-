#version 330

layout(location = 0) out vec4 positionMetallic;
layout(location = 1) out vec4 albedoRoughness;
layout(location = 2) out vec4 normalLit;
layout(location = 3) out vec4 emissionExtra;

in vec3 position_FS;
in vec2 texture_FS;
in vec3 normal_FS;

in vec3 view;

struct Material
{
	sampler2D albedoMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;

    vec3 colour;
	float metallic;
	float roughness;
};

uniform Material material;
uniform vec3 cameraPosition;

vec3 getColour(sampler2D map, vec3 alternateValue, vec2 texCoords)
{
	if(alternateValue.r == -1 || alternateValue.g == -1 || alternateValue.b == -1)
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
	vec3 albedoColour = pow(getColour(material.albedoMap, material.colour, texture_FS), vec3(2.2));
	
	float metallic = getColour(material.metallicMap, material.metallic, texture_FS).r;
	float roughness = pow(getColour(material.roughnessMap, material.roughness, texture_FS).r, 1.0);
	
	float dist = length(cameraPosition - worldPos_FS);

	vec3 N = normalize(normal_FS);

	positionMetallic = vec4(position_FS, metallic);
	albedoRoughness = vec4(albedoColour, roughness);
	normalLit = vec4(N, 1.0);
	emissionExtra = vec4(vec3(0.0), 1.0);
}