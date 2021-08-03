#version 430

layout(location = 0) out vec4 out_Bloom;

in vec2 texture_FS;

uniform sampler2D scene;

void main()
{
	out_Bloom = vec4(texture(scene, texture_FS).rgb, 1);
}