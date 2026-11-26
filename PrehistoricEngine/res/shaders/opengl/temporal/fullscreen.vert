#version 430
layout(location = 0) out vec2 uv;
void main()
{
    vec2 corner = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    uv = corner; gl_Position = vec4(corner * 2 - 1, 0, 1);
}
