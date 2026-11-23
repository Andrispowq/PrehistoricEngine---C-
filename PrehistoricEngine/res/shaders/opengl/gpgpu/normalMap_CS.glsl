#version 430

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) uniform writeonly image2D normalmap;

uniform sampler2D heightmap;
uniform int N;
uniform float strength;

#include "shared/terrain/heightNormal.glsl"

void main()
{
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
	vec3 normal = terrainHeightNormal(heightmap, vec2(pixel) / float(N), vec2(1.0 / float(N)), strength);
	imageStore(normalmap, pixel, vec4(normal * 0.5 + 0.5, 1));
}
