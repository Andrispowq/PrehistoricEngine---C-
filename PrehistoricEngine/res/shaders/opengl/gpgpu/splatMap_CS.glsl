#version 460

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba16f) uniform writeonly image2D splatmap;

uniform sampler2D normalmap;
uniform int N;

const float blendVal = 1;

void main(void)
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	vec2 texCoord = gl_GlobalInvocationID.xy / float(N);
	
	vec3 normal = texture(normalmap, texCoord).rgb * 2 - 1;
	normalize(normal);
	
	float slopeFactor = normal.z;
	
	vec4 blendValues = vec4(0, 0, 0, 0);
	
	if (slopeFactor > 0.5)
	{
		blendValues.x = blendVal;
	} else if (slopeFactor > 0.35)
	{
		blendValues.y = blendVal;
	} else 
	{
		blendValues.z = blendVal;
	}
	
	imageStore(splatmap, x, blendValues);
}