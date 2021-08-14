#version 430

layout(location = 0) out vec4 outColour;

in vec2 texture_FS;

uniform sampler2D scene;
uniform int horizontal;
uniform vec2 targetSize;

#define KERNEL_SIZE 4

/*0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216*/
uniform float weights[KERNEL_SIZE] = float[](0.07130343198685299, 0.13151412084312236, 0.1898792328888381, 0.214606428562373);

void main()
{
	vec2 texelSize = 1.0 / targetSize;

	vec3 combined = vec3(0);
	combined = texture(scene, texture_FS).rgb * weights[KERNEL_SIZE - 1];

	if(horizontal == 1)
	{	
        for(int i = 1; i < KERNEL_SIZE; ++i)
        {
            combined += texture(scene, texture_FS + vec2(texelSize.x * i, 0.0)).rgb * weights[(KERNEL_SIZE - 1) - i];
            combined += texture(scene, texture_FS - vec2(texelSize.x * i, 0.0)).rgb * weights[(KERNEL_SIZE - 1) - i];
        }
	}
	else
	{
		for(int i = 1; i < KERNEL_SIZE; ++i)
        {
            combined += texture(scene, texture_FS + vec2(0.0, texelSize.y * i)).rgb * weights[(KERNEL_SIZE - 1) - i];
            combined += texture(scene, texture_FS - vec2(0.0, texelSize.y * i)).rgb * weights[(KERNEL_SIZE - 1) - i];
        }
	}

    outColour = vec4(combined, 1.0);
}