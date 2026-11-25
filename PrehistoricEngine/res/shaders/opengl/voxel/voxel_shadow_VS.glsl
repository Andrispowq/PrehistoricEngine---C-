#version 450 core

layout(location = 0) in vec3 position;
uniform mat4 m_transform;
uniform mat4 lightViewProjection;

void main()
{
    gl_Position = lightViewProjection * m_transform * vec4(position, 1.0);
}
