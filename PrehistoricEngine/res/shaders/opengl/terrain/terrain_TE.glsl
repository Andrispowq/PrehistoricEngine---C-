#version 430

layout (quads, fractional_odd_spacing, ccw) in;

in vec2 mapCoord_TE[];

out vec2 mapCoord_GS;

uniform sampler2D heightmap;
uniform float scaleY;

#include "shared/terrain/quadEvaluation.glsl"
void main()
{
	vec4 position;
	vec2 mapCoord;
	evaluateTerrainQuad(position, mapCoord);
	float height = texture(heightmap, mapCoord).r * scaleY;

	position.y = height;
	gl_Position = position;
	
	mapCoord_GS = mapCoord;
}
