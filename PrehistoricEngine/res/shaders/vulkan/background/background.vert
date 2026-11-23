#version 450

layout(location = 0) out vec3 direction;

layout(push_constant) uniform Background
{
    mat4 inverseViewProjection;
} sky;

void main()
{
    vec2 position = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2) * 2.0 - 1.0;
    vec4 ray = sky.inverseViewProjection * vec4(position, 1.0, 1.0);
    direction = ray.xyz / ray.w;
    gl_Position = vec4(position, 1.0, 1.0);
}
