#version 430

layout (location = 0) in vec3 position_VS;
layout (location = 0) out vec3 position_FS;

uniform mat4 m_view;
uniform mat4 m_projection;

void main()
{
	gl_Position = m_projection * m_view * vec4(position_VS, 1);	
	position_FS = normalize(position_VS);
}