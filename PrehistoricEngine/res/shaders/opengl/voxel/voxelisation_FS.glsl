#version 460

// Lighting settings.
#define MAX_LIGHTS 1

// Lighting attenuation factors.
#define DIST_FACTOR 1.1f /* Distance is multiplied by this when calculating attenuation. */
#define CONSTANT 1
#define LINEAR 0
#define QUADRATIC 1

// Returns an attenuation factor given a distance.
float attenuate(float dist)
{ 
	dist *= DIST_FACTOR; 
	return 1.0f / (CONSTANT + LINEAR * dist + QUADRATIC * dist * dist); 
}

struct PointLight 
{
	vec3 position;
	vec3 colour;
	float intensity;
};

struct Material 
{
	vec3 diffuseColour;
	vec3 specularColour;
	float diffuseReflectivity;
	float specularReflectivity;
	float emissivity;
	float transparency;
};

uniform Material material;
uniform PointLight pointLights[MAX_LIGHTS];
uniform int numberOfLights;
uniform vec3 cameraPosition;
uniform float farPlane;

layout(RGBA8) uniform image3D texture3D;

in vec3 position_FS;
in vec3 normal_FS;

vec3 calculatePointLight(const PointLight light)
{
	const vec3 direction = normalize(light.position - position_FS);
	const float distanceToLight = distance(light.position, position_FS);
	const float attenuation = attenuate(distanceToLight);
	const float d = max(dot(normalize(normal_FS), direction), 0.0f);
	return d * light.intensity * attenuation * light.colour;
}

vec3 scaleAndBias(vec3 p) 
{ 
	return 0.5f * p + vec3(0.5f); 
}

bool isInsideCube(const vec3 p, float e) 
{ 
	return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e; 
}

void main()
{
	vec3 color = vec3(1.0f);
	
	vec3 pos_n = normalize(position_FS);
	if(!isInsideCube(pos_n, 0)) 
		return;

	// Calculate diffuse lighting fragment contribution.
	const uint maxLights = min(numberOfLights, MAX_LIGHTS);
	for(uint i = 0; i < maxLights; ++i) 
	{
		color += calculatePointLight(pointLights[i]);
	}
	
	vec3 spec = material.specularReflectivity * material.specularColour;
	vec3 diff = material.diffuseReflectivity * material.diffuseColour;
	color = (diff + spec) * color + material.emissivity * material.diffuseColour;

	// Output lighting to 3D texture.
	vec3 voxel = scaleAndBias(pos_n);
	ivec3 dim = imageSize(texture3D);
	float alpha = pow(1 - material.transparency, 4); // For soft shadows to work better with transparent materials.
	vec4 res = alpha * vec4(vec3(color), 1);
    imageStore(texture3D, ivec3(dim * voxel), res);
}