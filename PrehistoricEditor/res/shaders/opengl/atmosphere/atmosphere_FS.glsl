#version 430

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outPositionMetallic;
layout(location = 2) out vec4 outAlbedoRoughness;
layout(location = 3) out vec4 outNormal;
layout(location = 4) out vec4 outBloom;

in vec3 position_FS;

const vec3 baseColor = vec3(0.18, 0.27, 0.47);  //original color : 0.18, 0.27, 0.47

void main()
{
	float red = -0.00022 * (abs(position_FS.y) - 2800) + baseColor.x;
	float green = -0.00025 * (abs(position_FS.y) - 2800) + baseColor.y;
	float blue = -0.00019 * (abs(position_FS.y) - 2800) + baseColor.z;

	outColour = vec4(vec3(red, green, blue), 0.0);
	outPositionMetallic = vec4(position_FS, 0);
	outAlbedoRoughness = vec4(red, green, blue, 0.0);
	outNormal = vec4(-normalize(position_FS), 0);
	outBloom = vec4(0.0);
}