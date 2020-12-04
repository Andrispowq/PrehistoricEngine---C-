#version 330

layout (location = 0) in vec3 position_VS;

out vec3 position_FS;

uniform mat4 m_transform;
uniform mat4 m_view;
uniform mat4 m_projection;

void main()
{
	gl_Position = m_projection * m_view * m_transform * vec4(position_VS, 1);
	position_FS = (m_transform * vec4(position_VS, 1)).xyz;
}




