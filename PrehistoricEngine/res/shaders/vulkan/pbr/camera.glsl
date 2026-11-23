layout(set = 0, binding = 0, std140) uniform Camera
{
	layout(offset = 0) mat4 m_view;
	layout(offset = 64) mat4 m_projection;
	layout(offset = 128) vec3 cameraPosition;
};
