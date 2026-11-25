#version 450

layout(location = 0) in vec3 position;
layout(push_constant) uniform ShadowCaster { mat4 lightFromModel; };

void main()
{
    gl_Position = lightFromModel * vec4(position, 1.0);
    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;
}
