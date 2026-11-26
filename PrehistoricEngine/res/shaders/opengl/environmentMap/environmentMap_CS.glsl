#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform writeonly imageCube environmentMap;
layout(binding = 1, rgba32f) uniform readonly image2D equirectangularMap;

uniform vec2 equirectangularMapResolution;
uniform float resolution;
uniform mat4 worldToEnvironment;

#include "shared/environmentMap/cubeDirection.glsl"

void main()
{
    ivec3 pixel = ivec3(gl_GlobalInvocationID);
    vec3 direction = CubeDirection(pixel, int(resolution));
    ivec2 texel = PanoramaTexel(mat3(worldToEnvironment) * direction, ivec2(equirectangularMapResolution));
    imageStore(environmentMap, pixel, imageLoad(equirectangularMap, texel));
}
