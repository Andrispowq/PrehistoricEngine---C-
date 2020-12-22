#version 330

layout(location = 0) in vec2 position_VS;

out vec2 texture_FS;

void main()
{
	gl_Position = vec4(position_VS, 0, 1);

	texture_FS = (position_VS + 1) / 2;
}