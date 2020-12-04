#version 330 

layout (location = 0) in vec2 position_VS;

out vec2 texture_FS;

uniform mat4 m_transform;

void main()
{
	gl_Position = m_transform * vec4(position_VS, 0, 1);
	
	texture_FS = (position_VS + 1) / 2;
}