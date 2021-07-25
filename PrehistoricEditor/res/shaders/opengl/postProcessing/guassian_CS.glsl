#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;
uniform vec2 screenSize;

#define KERNEL_SIZE 5
#define SIGMA 10

uniform int horizontal;
uniform float weight[KERNEL_SIZE] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{

    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = gl_GlobalInvocationID.xy / screenSize;
	vec2 texelSize = 1.0 / screenSize * 3;
	
	vec3 combined = vec3(0);
	combined = texture(scene, texCoord).rgb * weight[0];
	
	if(horizontal == 1)
	{	
        for(int i = 1; i < KERNEL_SIZE; ++i)
        {
            combined += texture(scene, texCoord + vec2(texelSize.x * i, 0.0)).rgb * weight[i];
            combined += texture(scene, texCoord - vec2(texelSize.x * i, 0.0)).rgb * weight[i];
        }
	}
	else
	{
		for(int i = 1; i < 5; ++i)
        {
            combined += texture(scene, texCoord + vec2(0.0, texelSize.y * i)).rgb * weight[i];
            combined += texture(scene, texCoord - vec2(0.0, texelSize.y * i)).rgb * weight[i];
        }
	}

    imageStore(outColour, x, vec4(combined, 1.0));
}