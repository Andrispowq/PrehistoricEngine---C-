#version 460

layout(location = 0) in vec3 position_VS;
layout(location = 1) in vec2 texture_VS;
layout(location = 2) in vec3 normal_VS;
layout(location = 3) in vec3 tangent_VS;

out vec3 position_GS;
out vec3 normal_GS;

uniform mat4 m_transform;
uniform mat4 m_view;
uniform mat4 m_projection;

void main()
{
	vec4 worldPosition = m_transform * vec4(position_VS, 1.0);
	vec4 positionViewSpace = m_view * worldPosition;
	gl_Position = m_projection * positionViewSpace;

	position_GS = worldPosition.xyz;
	normal_GS = normalize(mat3(transpose(inverse(m_transform))) * normal_VS);
}