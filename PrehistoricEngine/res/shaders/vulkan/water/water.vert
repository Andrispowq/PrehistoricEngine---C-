#version 450
#extension GL_GOOGLE_include_directive : require
#include "../pbr/camera.glsl"
#include "../pbr/model.glsl"
#include "../../shared/terrain/patchMorph.glsl"
layout(location=0) in vec3 position;
layout(location=0) out vec2 mapCoord_TC;
struct Patch { vec4 locationGapMorph; vec4 quadrant; };
layout(set=2,binding=1,std430) readonly buffer WaterLeaves { Patch leaves[]; };
void main()
{
    Patch leaf = leaves[gl_InstanceIndex];
    vec2 location = leaf.locationGapMorph.xy;
    float gap = leaf.locationGapMorph.z;
    vec2 uv = location + gap * position.xz;
    if (leaf.quadrant.z > 0)
        uv += morph(uv, 0, leaf.locationGapMorph.w, location, leaf.quadrant.xy, gap, m_transform, cameraPosition);
    mapCoord_TC = uv;
    gl_Position = m_transform * vec4(uv.x, 0, uv.y, 1);
}
