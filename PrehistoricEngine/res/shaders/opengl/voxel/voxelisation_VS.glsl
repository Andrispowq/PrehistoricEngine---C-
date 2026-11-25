#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

uniform mat4 m_transform;
out Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
} vertex;

void main()
{
    vertex.position = (m_transform * vec4(position, 1.0)).xyz;
    vertex.normal = transpose(inverse(mat3(m_transform))) * normal;
    vertex.uv = uv;
    gl_Position = vec4(vertex.position, 1.0);
}
