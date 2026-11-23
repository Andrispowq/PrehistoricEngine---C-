#version 460

layout (location = 0) out vec4 outColour;
layout (location = 1) out vec4 outPositionMetallic;
layout (location = 2) out vec4 outAlbedoRoughness;
layout (location = 3) out vec4 outNormal;
layout (location = 4) out vec4 outBloom;


void main()
{
	outColour = vec4(.07, .08, .9, 1);
	outPositionMetallic = vec4(0.0);
	outAlbedoRoughness = vec4(0.0);
	outNormal = vec4(0.0);
	outBloom = vec4(0.0);
}