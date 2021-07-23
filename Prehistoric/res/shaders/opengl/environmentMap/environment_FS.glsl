#version 430

layout (location = 0) out vec4 outColour;
layout (location = 1) out vec4 outBloom;

layout (location = 0) in vec3 position_FS;

uniform samplerCube environmentMap;
uniform float lod;

uniform float threshold;

void main()
{
	vec3 colour = textureLod(environmentMap, normalize(position_FS), lod).rgb;

	vec3 bloom = colour;
	float brightness = dot(colour.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness < threshold)
	{
		bloom = vec3(0.0);
	}

	outColour = vec4(colour, 1);
	outBloom = vec4(bloom, 1);
}