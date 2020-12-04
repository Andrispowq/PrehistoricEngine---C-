#version 330 

layout (location = 0) out vec4 outColour;

in vec2 texture_FS;
in float distanceFromLeft;

uniform sampler2D image;

uniform float progress;

void main()
{
	outColour = texture(image, texture_FS) * progress;
}