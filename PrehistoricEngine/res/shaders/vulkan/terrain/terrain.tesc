#version 450
#extension GL_GOOGLE_include_directive : require
#include "../pbr/camera.glsl"
#include "params.glsl"
layout (vertices = 16) out;

layout(location=0) in vec2 mapCoord_TC[];

layout(location=0) out vec2 mapCoord_TE[];

const int AB = 2;
const int BC = 3;
const int CD = 0;
const int DA = 1;

#include "../../shared/terrain/tessellation.glsl"
float LodFactor(float dist)
{
	return terrainLodFactor(dist, tessellation.x, tessellation.y, tessellation.z);
}
#include "../../shared/terrain/quadControl.glsl"
