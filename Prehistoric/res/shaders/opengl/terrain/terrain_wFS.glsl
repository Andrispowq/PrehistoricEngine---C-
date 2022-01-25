#version 430

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outBloom;

void main()
{
	outColour = vec4(vec3(0.1, 1.0, 0.1), 0.0);
	outBloom = vec4(0.0);
}