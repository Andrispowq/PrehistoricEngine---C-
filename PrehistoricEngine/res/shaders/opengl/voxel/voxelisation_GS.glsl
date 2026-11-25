#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
} vertices[];

out Fragment
{
    vec3 position;
    vec3 normal;
    vec2 uv;
} fragment;

uniform vec3 volumeMin;
uniform float volumeExtent;

void main()
{
    vec3 axis = abs(cross(vertices[1].position - vertices[0].position,
                          vertices[2].position - vertices[0].position));
    for (int i = 0; i < 3; ++i)
    {
        fragment.position = vertices[i].position;
        fragment.normal = vertices[i].normal;
        fragment.uv = vertices[i].uv;
        vec3 p = 2.0 * (fragment.position - volumeMin) / volumeExtent - 1.0;
        vec2 projected = axis.x >= max(axis.y, axis.z) ? p.yz : (axis.y >= axis.z ? p.xz : p.xy);
        gl_Position = vec4(projected, 0.0, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}
