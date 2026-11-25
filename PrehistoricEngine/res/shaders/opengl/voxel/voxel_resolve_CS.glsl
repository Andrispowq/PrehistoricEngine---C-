#version 450 core

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout(binding = 0, r32ui) uniform readonly uimage3D packedVoxels;
layout(binding = 1, rgba16f) uniform writeonly image3D radianceVolume;

void main()
{
    // The supported grid dimensions are multiples of the workgroup dimensions.
    ivec3 cell = ivec3(gl_GlobalInvocationID);
    uint encodedVoxel = imageLoad(packedVoxels, cell).r;
    vec3 radiance = exp2(unpackUnorm4x8(encodedVoxel).rgb * 8.0) - 1.0;
    imageStore(radianceVolume, cell, vec4(radiance, encodedVoxel != 0u ? 1.0 : 0.0));
}
