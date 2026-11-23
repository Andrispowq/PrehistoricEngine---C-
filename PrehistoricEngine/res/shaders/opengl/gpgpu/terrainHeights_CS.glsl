#version 430

layout (local_size_x = 16, local_size_y = 16) in;

layout (std430, binding = 0) writeonly buffer TerrainHeights
{
	float heights[];
}; 

uniform sampler2D heightmap;
uniform int N;

void main()
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	vec2 texCoord = gl_GlobalInvocationID.xy / float(N);
	
	heights[x.y * N + x.x] = texture(heightmap, texCoord).r;
};