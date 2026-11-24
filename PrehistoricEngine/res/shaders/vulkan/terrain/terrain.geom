#version 450
#extension GL_GOOGLE_include_directive : require
#include "../water/capture.glsl"
#include "../pbr/camera.glsl"
#include "detail.glsl"
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;
layout(location=0) in vec2 mapCoord_GS[];
layout(location=0) out vec3 position_FS;
layout(location=1) out vec2 texture_FS;
layout(location=2) out vec3 basePosition;
layout(location=3) out vec3 tangent_FS;

void emitTerrainVertex(int i, float offset, vec3 tangent)
{
	vec4 p = gl_in[i].gl_Position;
	basePosition = p.xyz;
	p.y += offset;
	texture_FS = mapCoord_GS[i];
	position_FS = p.xyz;
	tangent_FS = tangent;
	gl_Position = sceneClip(m_projection, m_view, p);
	EmitVertex();
}

#define highDetailRange heightScales.w
#include "../../shared/terrain/geometry.glsl"
