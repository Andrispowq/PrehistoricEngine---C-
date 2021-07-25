#version 430

layout(location = 0) out vec4 bloom0;
layout(location = 1) out vec4 bloom1;
layout(location = 2) out vec4 bloom2;
layout(location = 3) out vec4 bloom3;
layout(location = 4) out vec4 bloom4;

in vec2 texture_FS;

uniform sampler2D scene;

void main()
{
	vec3 colour = texture(scene, texture_FS).rgb;
	
	bloom0 = vec4(colour, 1);//vec4(texture_FS, 0, 1);
	bloom1 = vec4(colour, 1);//vec4(texture_FS, 0, 1);
	bloom2 = vec4(colour, 1);//vec4(texture_FS, 0, 1);
	bloom3 = vec4(colour, 1);//vec4(texture_FS, 0, 1);
	bloom4 = vec4(colour, 1);//vec4(texture_FS, 0, 1);
}