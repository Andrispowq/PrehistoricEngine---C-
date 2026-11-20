#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position_VS;
layout(location = 1) in vec2 texture_VS;
layout(location = 2) in vec3 normal_VS;
layout(location = 3) in vec3 tangent_VS;

layout(location = 0) out vec3 position_FS;
layout(location = 1) out vec2 texture_FS;
layout(location = 2) out vec3 normal_FS;

layout(set = 0, binding = 0) uniform Camera 
{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
};

layout(set = 2, binding = 0) uniform Model
{
    mat4 model;
};

void main() 
{
    vec4 worldPos = model * vec4(position_VS, 1.0);
    gl_Position = proj * view * worldPos;

	position_FS = worldPos.xyz;
	texture_FS = texture_VS;
	normal_FS = (model * vec4(normal_VS, 0)).xyz;
}