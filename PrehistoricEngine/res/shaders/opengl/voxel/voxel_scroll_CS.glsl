#version 450 core
layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout(binding = 0, r32ui) uniform readonly uimage3D sourceCells;
layout(binding = 1, r32ui) uniform writeonly uimage3D destinationCells;
uniform vec3 scrollDelta;
#include "shared/voxel/scroll.glsl"
void main()
{
    ivec3 cell = ivec3(gl_GlobalInvocationID);
    int size = imageSize(destinationCells).x;
    if (any(greaterThanEqual(cell, ivec3(size)))) return;
    ivec3 delta = ivec3(scrollDelta);
    uint value = retainedVoxel(cell, delta, size) ? imageLoad(sourceCells, cell + delta).r : 0u;
    imageStore(destinationCells, cell, uvec4(value));
}
