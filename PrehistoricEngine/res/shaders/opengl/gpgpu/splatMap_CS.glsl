#version 460

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba16f) uniform writeonly image2D splatmap;

uniform sampler2D normalmap;
uniform int N;

#include "shared/terrain/layers.glsl"

void main(void)
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	vec2 texCoord = gl_GlobalInvocationID.xy / float(N);
	
	vec3 normal = texture(normalmap, texCoord).rgb * 2 - 1;
	imageStore(splatmap, x, vec4(terrainLayerWeights(normal.z), 0));
}
