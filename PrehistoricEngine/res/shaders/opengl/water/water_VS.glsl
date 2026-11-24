#version 430

layout (location = 0) in vec2 position_VS;

out vec2 mapCoord_TC;

uniform mat4 localMatrix;
uniform mat4 worldMatrix;
uniform vec3 cameraPosition;
uniform vec2 location;
uniform vec2 index;
uniform float gap;
uniform int lodMorphArea[8];
uniform int lod;

#include "shared/terrain/patchMorph.glsl"

void main()
{
	vec2 localPosition = (localMatrix * vec4(position_VS.x, 0, position_VS.y, 1)).xz;
	
	if (lod > 0)
		localPosition += morph(localPosition, 0.0, lodMorphArea[lod - 1], location, index, gap, worldMatrix, cameraPosition);
		
	mapCoord_TC = localPosition;	
	gl_Position = worldMatrix * vec4(localPosition.x, 0, localPosition.y, 1);
}