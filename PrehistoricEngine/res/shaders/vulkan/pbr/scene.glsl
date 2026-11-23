layout (location = 0) out vec4 outColour;
layout(location = 1) out vec4 outBloom;

const int max_lights = 1000;
const float PI = 3.141592653589793;
const float emissionFactor = 3.0;

layout (location = 0) in vec3 position_FS;
layout (location = 1) in vec2 texture_FS;
layout (location = 2) in vec3 normal_FS;
layout (location = 3) in vec3 tangent_FS;

#include "camera.glsl"

layout(set = 0, binding = 1, std140) uniform LightConditions
{
	layout(offset = 0) int highDetailRange;
	layout(offset = 4) int numberOfLights;
};

layout(set = 0, binding = 2, std140) uniform Lights
{
	layout(offset = max_lights * 0 * 16) vec4 position[max_lights];
	layout(offset = max_lights * 1 * 16) vec4 colour[max_lights];
	layout(offset = max_lights * 2 * 16) vec4 intensity[max_lights];
} lights;

layout (set = 0, binding = 3) uniform samplerCube irradianceMap;
layout (set = 0, binding = 4) uniform samplerCube prefilterMap;
layout (set = 0, binding = 5) uniform sampler2D brdfLUT;

layout(set = 0, binding = 6, std140) uniform Environment
{
	layout(offset = 0) float maxReflectionLOD;
};

#include "../../shared/pbr/lighting.glsl"
#include "shade.glsl"

