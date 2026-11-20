#version 430

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) uniform writeonly image2D outBloom;

uniform sampler2D scene;
uniform float threshold;
uniform vec2 targetSize;

void main()
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = gl_GlobalInvocationID.xy / targetSize;

	vec3 bloomColour = texture(scene, texCoord).rgb;
	float intensity = dot(bloomColour, vec3(0.2126, 0.7152, 0.0722));
	if(intensity < threshold)
	{
		bloomColour = vec3(0.0);
	}

	bloomColour = clamp(bloomColour, 0.0, 500.0);

	imageStore(outBloom, x, vec4(bloomColour, 1.0));
}
	