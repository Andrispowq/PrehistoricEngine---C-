#version 450

layout(location = 0) out vec3 direction;

layout(push_constant) uniform Background
{
    vec4 rayX;
    vec4 rayY;
    vec4 rayCenter;
} sky;

void main()
{
    vec2 position = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2) * 2.0 - 1.0;
    direction = sky.rayX.xyz * position.x + sky.rayY.xyz * position.y + sky.rayCenter.xyz;
    gl_Position = vec4(position, 1.0, 1.0);
}
