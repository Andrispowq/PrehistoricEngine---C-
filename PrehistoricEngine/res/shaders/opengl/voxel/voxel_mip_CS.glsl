#version 450 core

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout(binding = 0, rgba16f) uniform readonly image3D source;
layout(binding = 1, rgba16f) uniform writeonly image3D destination;

void main()
{
    ivec3 cell = ivec3(gl_GlobalInvocationID);
    if (any(greaterThanEqual(cell, imageSize(destination)))) return;

    vec4 sum = vec4(0);
    for (int z = 0; z < 2; ++z)
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 2; ++x)
                sum += imageLoad(source, 2 * cell + ivec3(x, y, z));

    imageStore(destination, cell, sum * 0.125);
}
