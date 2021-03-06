#version 430

layout (location = 0) out vec4 positionMetallic;
layout (location = 1) out vec4 albedoRoughness;
layout (location = 2) out vec4 normalLit;
layout (location = 3) out vec4 emissionExtra;

layout (location = 0) in vec3 position_FS;

uniform samplerCube environmentMap;
uniform float lod;

void main()
{
	vec3 envColour = textureLod(environmentMap, normalize(position_FS), lod).rgb;

	positionMetallic = vec4(vec3(0.0), 0.0);
	albedoRoughness = vec4(envColour, 0.0);
	normalLit = vec4(vec3(0.0), 0.2);
	emissionExtra = vec4(vec3(0.0), 0.0);
}