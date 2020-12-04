#version 330

layout(location = 0) in vec3 position_VS;
layout(location = 1) in vec2 texture_VS;
layout(location = 2) in vec3 normal_VS;

out vec3 worldPos_FS;
out vec2 texture_FS;
out vec3 normal_FS;

uniform mat4 m_transform;
uniform mat4 m_view;
uniform mat4 m_projection;

void main()
{
	vec4 worldPos = m_transform * vec4(position_VS, 1);
	gl_Position = m_projection * m_view * worldPos;

	worldPos_FS = worldPos.xyz;
	texture_FS = texture_VS;
	normal_FS = (m_transform * vec4(normal_VS, 0)).xyz;
}