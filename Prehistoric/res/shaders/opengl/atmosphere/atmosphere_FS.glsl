#version 430

layout(location = 0) out vec4 positionMetallic;
layout(location = 1) out vec4 albedoRoughness;
layout(location = 2) out vec4 normalLit;
layout(location = 3) out vec4 emissionExtra;

in vec3 position_FS;

const vec3 baseColor = vec3(0.18, 0.27, 0.47);  //original color : 0.18, 0.27, 0.47

void main()
{
	float red = -0.00022 * (abs(position_FS.y) - 2800) + baseColor.x;
	float green = -0.00025 * (abs(position_FS.y) - 2800) + baseColor.y;
	float blue = -0.00019 * (abs(position_FS.y) - 2800) + baseColor.z;

	positionMetallic = vec4(vec3(position_FS), 0.0);
	albedoRoughness = vec4(vec3(red, green, blue), 0.0);
	normalLit = vec4(vec3(0.0), 0.2);
	emissionExtra = vec4(vec3(0.0), 0.0);
}