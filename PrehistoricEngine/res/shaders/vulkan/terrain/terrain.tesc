#version 450
#extension GL_GOOGLE_include_directive : require
#include "patch.glsl"
#include "params.glsl"
#include "../../shared/terrain/tessellation.glsl"
layout(vertices=3) out;

float edgeLevel(int a, int b)
{
	vec4 midpoint = 0.5 * (gl_in[a].gl_Position + gl_in[b].gl_Position);
	midpoint.y = heightAt(midpoint.xz + 0.5);
	float d = distance((m_transform * midpoint).xyz, cameraPosition);
	return mix(1.0, float(gl_MaxTessGenLevel), terrainLodFactor(d, tessellation.x, tessellation.y, tessellation.z));
}

void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = edgeLevel(1, 2);
		gl_TessLevelOuter[1] = edgeLevel(2, 0);
		gl_TessLevelOuter[2] = edgeLevel(0, 1);
		gl_TessLevelInner[0] = max(gl_TessLevelOuter[0], max(gl_TessLevelOuter[1], gl_TessLevelOuter[2]));
	}
}
