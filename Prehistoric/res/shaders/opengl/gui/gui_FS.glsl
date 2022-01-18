#version 330 

layout (location = 0) out vec4 outColour;

in vec2 texture_FS;
in float distanceFromLeft;

uniform vec3 colour;
uniform int texType;

uniform sampler2D image;

void main()
{
	outColour = vec4(colour, 1.0);

	if (outColour.r == -1)
	{
		outColour = texture(image, texture_FS);
	}
}