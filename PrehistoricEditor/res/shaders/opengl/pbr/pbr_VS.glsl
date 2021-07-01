#version 430

layout(location = 0) in vec3 position_VS;
layout(location = 1) in vec2 texture_VS;
layout(location = 2) in vec3 normal_VS;
layout(location = 3) in vec3 tangent_VS;

const int max_lights = 10;

out vec3 position_FS;
out vec2 texture_FS;
out vec3 normal_FS;
out vec3 tangent_FS;

uniform mat4 m_transform;
uniform mat4 m_view;
uniform mat4 m_projection;

void main()
{
	vec4 worldPosition = m_transform * vec4(position_VS, 1.0);
	gl_Position = m_projection * m_view * worldPosition;

	vec3 N = normalize((m_transform * vec4(normal_VS, 0.0)).xyz);
	vec3 T = normalize((m_transform * vec4(tangent_VS, 0.0)).xyz);
	T = normalize(T - dot(T, N) * N); // re-orthogonalise T with respect to N

	position_FS = worldPosition.xyz;
	texture_FS = texture_VS;
	normal_FS = N;
	tangent_FS = T;
}