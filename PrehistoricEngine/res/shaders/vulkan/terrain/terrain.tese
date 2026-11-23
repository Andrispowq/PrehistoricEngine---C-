#version 450
#extension GL_GOOGLE_include_directive : require
#include "evaluate.glsl"
#include "detail.glsl"
layout(location=0) out vec3 position_FS;
layout(location=1) out vec2 texture_FS;
layout(location=2) out vec3 basePosition;
layout(location=3) out vec3 tangent_FS;
void main()
{
	vec4 p;
	evaluateSurface(p, texture_FS);
	basePosition = p.xyz;
	float dist = distance(cameraPosition, p.xyz);
	if (dist < heightScales.w)
		p.y += terrainOffset(texture_FS, dist);
	tangent_FS = vec3(0);
	position_FS = p.xyz;
	gl_Position = m_projection * m_view * p;
}
