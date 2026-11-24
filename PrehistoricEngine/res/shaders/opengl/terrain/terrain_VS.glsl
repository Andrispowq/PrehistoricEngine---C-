#version 430

layout (location = 0) in vec2 position_VS;

out vec2 mapCoord_TC;

uniform sampler2D heightmap;
uniform mat4 worldMatrix;
uniform mat4 localMatrix;
uniform vec3 cameraPosition;
uniform vec2 location;
uniform vec2 index;
uniform float gap;
uniform int lodMorphArea[8];
uniform int lod;

float heightAt(vec2 uv)
{
	return textureLod(heightmap, uv, 0).r;
}

#include "shared/terrain/patchMorph.glsl"
#include "shared/terrain/morph.glsl"

void main()
{
	vec2 localPosition = (localMatrix * vec4(position_VS.x, 0, position_VS.y, 1)).xz;

	float height = heightAt(localPosition);

	if (lod > 0)
		morphTerrainEndpoint(localPosition, height, lodMorphArea[lod - 1], location, index, gap, worldMatrix, cameraPosition);

	mapCoord_TC = localPosition;
	gl_Position = worldMatrix * vec4(localPosition.x, height, localPosition.y, 1);
}
