#version 430

layout (location = 0) in vec3 position_VS;

uniform mat4 m_transform;

void main()
{
	gl_Position = m_transform * vec4(position_VS, 1.0);
}