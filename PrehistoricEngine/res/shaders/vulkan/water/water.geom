#version 450
#extension GL_GOOGLE_include_directive : require
#include "../pbr/camera.glsl"
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;
layout(location=0) in vec4 positions[];
layout(location=1) in vec2 coords[];
layout(location=0) out vec4 world;
layout(location=1) out vec2 waveUV;
void main()
{
    for (int i=0; i<3; ++i)
    {
        world = positions[i];
        waveUV = coords[i];
        gl_Position = m_projection * m_view * world;
        EmitVertex();
    }
    EndPrimitive();
}
