#version 450

layout (location = 0) in vec3 position_VS;
layout (location = 1) in vec2 texture_VS;
layout (location = 2) in vec3 normal_VS;
layout (location = 3) in vec3 tangent_VS;

layout (location = 0) out vec3 position_FS;
layout (location = 1) out vec2 texture_FS;
layout (location = 2) out vec3 normal_FS;
layout (location = 3) out vec3 tangent_FS;

layout(set = 0, binding = 0, std140) uniform Camera
{
	layout(offset = 0) mat4 m_view;
	layout(offset = 64) mat4 m_projection;
	layout(offset = 128) vec3 cameraPosition;
};

layout(set = 2, binding = 0) uniform Model
{
	layout(offset = 0) mat4 m_transform;
};

void main()
{
	vec4 worldPosition = m_transform * vec4(position_VS, 1.0);
	gl_Position = m_projection * m_view * worldPosition;
	
	vec3 N = normalize((m_transform * vec4(normal_VS, 0.0)).xyz);
	vec3 T = normalize((m_transform * vec4(tangent_VS, 0.0)).xyz);
	T = normalize(T - dot(T, N) * N); // re-orthogonalise T with respect to N
	vec3 B = normalize(cross(N, T));

	if (isnan(T.x))
	{
		T = vec3(0);
	}

	position_FS = worldPosition.xyz;
	texture_FS = texture_VS;
	normal_FS = N;
	tangent_FS = T;
}