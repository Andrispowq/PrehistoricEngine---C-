#version 450 core

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout(binding = 0, rgba16f) uniform readonly image3D source;
layout(binding = 1, rgba16f) uniform writeonly image3D destination;
uniform int directionIndex;

void main()
{
    ivec3 cell = ivec3(gl_GlobalInvocationID);
    if (any(greaterThanEqual(cell, imageSize(destination)))) return;

    int axis = directionIndex / 2;
    int u = (axis + 1) % 3;
    int v = (axis + 2) % 3;
    int nearSide = directionIndex % 2;
    ivec3 offset = ivec3(0);
    offset[axis] = nearSide;
    vec4 sum = vec4(0.0);
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 2; ++x)
        {
            offset[u] = x;
            offset[v] = y;
            ivec3 nearCell = 2 * cell + offset;
            ivec3 farCell = nearCell;
            farCell[axis] += 1 - 2 * nearSide;
            vec4 front = imageLoad(source, nearCell);
            vec4 back = imageLoad(source, farCell);
            sum += front + (1.0 - front.a) * back;
        }
    imageStore(destination, cell, sum * 0.25);
}
