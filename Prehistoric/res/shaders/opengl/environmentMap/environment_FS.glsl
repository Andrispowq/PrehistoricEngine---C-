#version 430

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outPositionMetallic;
layout(location = 2) out vec4 outAlbedoRoughness;
layout(location = 3) out vec4 outNormal;
layout(location = 4) out vec4 outBloom;

layout (location = 0) in vec3 position_FS;

uniform samplerCube environmentMap;
uniform float lod;

uniform float threshold;

void main()
{
	vec3 colour = textureLod(environmentMap, normalize(position_FS), lod).rgb;
	outColour = vec4(colour, 1);
	outPositionMetallic = vec4(position_FS, 0);
	outAlbedoRoughness = vec4(colour, 0);
	outNormal = vec4(-normalize(position_FS), 0);
	outBloom = vec4(0.0);
}