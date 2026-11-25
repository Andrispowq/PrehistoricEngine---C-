#version 450 core

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout(binding = 0, r32ui) uniform readonly uimage3D source;
layout(binding = 1, r32ui) uniform writeonly uimage3D destination;

void main()
{
    ivec3 cell = ivec3(gl_GlobalInvocationID);
    if (any(greaterThanEqual(cell, imageSize(destination)))) return;

    uint occupied = 0u;
    for (int z = 0; z < 2; ++z)
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 2; ++x)
                occupied |= imageLoad(source, 2 * cell + ivec3(x, y, z)).r;

    imageStore(destination, cell, uvec4(occupied != 0u ? 1u : 0u));
}
