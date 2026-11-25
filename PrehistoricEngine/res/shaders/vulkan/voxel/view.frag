#version 450
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColour;
layout(std430, binding = 0) readonly buffer Occupancy { uint cells[]; };
layout(push_constant) uniform View { mat4 inverseViewProjection; vec4 grid; int resolution; };

#define volumeMin grid.xyz
#define volumeExtent grid.w

const float voxelNearClip = 0.0;

int voxelResolution() {
    return resolution;
}

uint readVoxel(ivec3 p) {
    return cells[p.x + resolution * (p.y + resolution * p.z)];
}

#include "../../shared/voxel/view.glsl"

void main() {
    outColour = vec4(drawVoxels(uv * 2.0 - 1.0), 1.0);
}
