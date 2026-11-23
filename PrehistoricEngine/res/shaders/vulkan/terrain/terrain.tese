#version 450
#extension GL_GOOGLE_include_directive : require
#include "patch.glsl"
layout(triangles, fractional_odd_spacing, cw) in;
layout(location=0) out vec3 position_FS;
layout(location=1) out vec2 texture_FS;
layout(location=2) out vec3 normal_FS;
layout(location=3) out vec3 tangent_FS;

void main()
{
	precise vec4 p = gl_TessCoord.x * gl_in[0].gl_Position + gl_TessCoord.y * gl_in[1].gl_Position + gl_TessCoord.z * gl_in[2].gl_Position;
	texture_FS = p.xz + 0.5;
	p.y = heightAt(texture_FS);
	vec2 stepUV = 1.0 / vec2(textureSize(heightMap, 0));
	float dx = heightAt(texture_FS + vec2(stepUV.x, 0)) - heightAt(texture_FS - vec2(stepUV.x, 0));
	float dz = heightAt(texture_FS + vec2(0, stepUV.y)) - heightAt(texture_FS - vec2(0, stepUV.y));
	vec3 tx = mat3(m_transform) * vec3(2.0 * stepUV.x, dx, 0);
	vec3 tz = mat3(m_transform) * vec3(0, dz, 2.0 * stepUV.y);
	tangent_FS = normalize(tx);
	normal_FS = normalize(cross(tz, tx));
	position_FS = (m_transform * p).xyz;
	gl_Position = m_projection * m_view * vec4(position_FS, 1);
}
