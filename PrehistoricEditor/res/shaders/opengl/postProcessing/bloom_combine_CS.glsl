#version 430

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D image0;
uniform sampler2D image1;
uniform vec2 targetSize;

void main()
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = gl_GlobalInvocationID.xy / targetSize;
	
	vec3 colour0 = texture(image0, texCoord).rgb;
    vec3 colour1 = texture(image1, texCoord).rgb;

	imageStore(outColour, x, vec4(colour0 + colour1, 1.0));
}