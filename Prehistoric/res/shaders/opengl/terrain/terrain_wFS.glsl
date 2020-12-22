#version 430

layout(location = 0) out vec4 positionMetallic;
layout(location = 1) out vec4 albedoRoughness;
layout(location = 2) out vec4 normalLit;
layout(location = 3) out vec4 emissionExtra;

in vec3 position_FS;

void main()
{
	positionMetallic = vec4(position_FS, 0.0);
	albedoRoughness = vec4(vec3(0.1, 1.0, 0.1), 0.0);
	normalLit = vec4(vec3(0.0), 0.0);
	emissionExtra = vec4(vec3(0.0), 0.0);
}