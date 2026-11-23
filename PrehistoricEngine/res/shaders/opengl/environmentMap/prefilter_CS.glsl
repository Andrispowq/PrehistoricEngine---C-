#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform writeonly imageCube prefilterMap;

uniform samplerCube environmentMap;
uniform float roughness;
uniform float resolution;

#include "opengl/environmentMap/shared/ggx.glsl"
#include "opengl/environmentMap/shared/convolution.glsl"
#include "opengl/environmentMap/shared/cubeDirection.glsl"

void main()
{
    ivec3 pixel = ivec3(gl_GlobalInvocationID);
    vec3 N = CubeDirection(pixel, int(resolution));
    vec3 colour = ConvolveEnvironment(N, roughness, 2048u, false);
    imageStore(prefilterMap, pixel, vec4(colour, 1.0));
}
