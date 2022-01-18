#version 430

layout (location = 0) out vec4 outColour;

layout (location = 0) in vec3 position_FS;

uniform samplerCube environmentMap;
uniform float lod;

uniform float threshold;

void main()
{
	vec3 colour = textureLod(environmentMap, normalize(position_FS), lod).rgb;
	outColour = vec4(colour, 1);
}