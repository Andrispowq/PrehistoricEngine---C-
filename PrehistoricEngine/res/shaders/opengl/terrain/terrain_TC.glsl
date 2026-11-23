#version 430

layout (vertices = 16) out;

in vec2 mapCoord_TC[];

out vec2 mapCoord_TE[];

const int AB = 2;
const int BC = 3;
const int CD = 0;
const int DA = 1;

uniform int tessellationFactor;
uniform float tessellationSlope;
uniform float tessellationShift;
uniform vec3 cameraPosition;

#include "shared/terrain/tessellation.glsl"

float LodFactor(float dist)
{
	return terrainLodFactor(dist, tessellationFactor, tessellationSlope, tessellationShift);
}

#include "shared/terrain/quadControl.glsl"
