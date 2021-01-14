#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2DMS positionMetallic;
uniform vec2 dimension;
uniform int sampleCount;

const float threshold = 1.0;

void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = gl_GlobalInvocationID.xy / dimension;

	float positionDiscontinuities = 0;
	vec3[8] positions;
	for (int i = 0; i < sampleCount; i++)
	{
		positions[i] = texelFetch(positionMetallic, x, i).rgb;
	}
	
	for (int i = 0; i < sampleCount - 1; i++)
	{
		positionDiscontinuities += distance(positions[i], positions[i + 1]);
	}
	
	float coverageValue = 0;
	
	if(positionDiscontinuities > threshold)
	{	
		coverageValue = 1.0;
	}
	
    imageStore(outColour, x, vec4(coverageValue, 0.0, 0.0, 1.0));
}