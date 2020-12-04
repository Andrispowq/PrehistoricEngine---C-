#version 430

layout (location = 0) in vec3 position_VS;
layout (location = 1) in vec2 texture_VS;

layout (location = 0) out vec2 texture_FS;

void main()
{
	gl_Position = vec4(position_VS, 1);
	
	texture_FS = texture_VS;
}