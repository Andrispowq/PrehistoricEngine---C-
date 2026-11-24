#version 450
#extension GL_GOOGLE_include_directive : require
#include "patch.glsl"
#include "../../shared/terrain/patchMorph.glsl"
#include "../../shared/terrain/morph.glsl"
layout(location=0) in vec3 position;
layout(location=0) out vec2 mapCoord_TC;
struct Patch { vec4 locationGapMorph; vec4 quadrant; };
layout(set=2, binding=3, std430) readonly buffer TerrainLeaves { Patch leaves[]; };
void main()
{
	Patch leaf = leaves[gl_InstanceIndex];
	vec2 location = leaf.locationGapMorph.xy;
	float gap = leaf.locationGapMorph.z;
	float morphArea = leaf.locationGapMorph.w;
	vec2 uv = location + gap * position.xz;
	float height = heightAt(uv);
	if (leaf.quadrant.z > 0)
		morphTerrainEndpoint(uv, height, morphArea, location, leaf.quadrant.xy, gap, m_transform, cameraPosition);
	mapCoord_TC = uv;
	gl_Position = m_transform * vec4(uv.x, height, uv.y, 1);
}
