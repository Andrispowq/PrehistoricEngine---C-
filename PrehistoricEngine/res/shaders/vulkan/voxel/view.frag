#version 450
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColour;
layout(std430, binding = 0) readonly buffer Occupancy { uint cells[]; };
layout(push_constant) uniform View { mat4 inverseViewProjection; vec4 grid; int resolution; };

#define volumeMin grid.xyz
#define volumeExtent grid.w

const float voxelNearClip = 0.0;

int voxelResolution() {
    return resolution & 65535;
}

uint readVoxel(ivec3 p) {
    int size = voxelResolution();
    uint value = cells[p.x + size * (p.y + size * p.z)];
    if (value == 0u || (resolution >> 16) == 0) return value;
    const vec3 colours[4] = vec3[4](vec3(1, .25, .15), vec3(.2, 1, .3), vec3(.2, .5, 1), vec3(1, .4, 1));
    return packUnorm4x8(vec4(colours[(resolution >> 16) - 1], 1));
}

#include "../../shared/voxel/view.glsl"

void main() {
    outColour = vec4(drawVoxels(uv * 2.0 - 1.0), 1.0);
}
