#version 430

layout (location = 0) in vec3 position_VS;

layout (location = 0) out vec3 worldPosition;

uniform mat4 m_transform;
uniform mat4 m_view;
uniform mat4 m_projection;

void main()
{
	vec4 positionWorld = m_transform * vec4(position_VS, 1);
	gl_Position = m_projection * m_view * positionWorld;
	
	worldPosition = positionWorld.xyz;
}