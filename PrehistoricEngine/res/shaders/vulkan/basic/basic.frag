#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable
#include "../water/capture.glsl"

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outBloom;

layout(location = 0) in vec3 position_FS;
layout(location = 1) in vec2 texture_FS;
layout(location = 2) in vec3 normal_FS;

const float PI = 3.141592653589793238;
const int max_lights = 10;

layout(set = 0, binding = 0) uniform Camera 
{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
};

layout (set = 0, binding = 1, std140) uniform Lights
{
    layout (offset = max_lights * 0 * 16) vec3 position[max_lights];
	layout (offset = max_lights * 1 * 16) vec3 colour[max_lights];
	layout (offset = max_lights * 2 * 16) vec3 intensity[max_lights];
} lights;

layout (set = 2, binding = 1, std140) uniform Materials
{
    layout (offset = 00) vec3 colour;	
	layout (offset = 16) vec4 mrot;
} material;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D mrotMap;

#include "../../shared/pbr/lighting.glsl"

void main() 
{
	vec3 albedoColour = material.colour;
	vec4 mrot = material.mrot;
	
	if (albedoColour.r == -1)
	{
		albedoColour = pow(texture(albedoMap, texture_FS).rgb, vec3(2.2));
	}
	
	vec4 mrotMap = texture(mrotMap, texture_FS);
	if (mrot.r == -1)
	{
		mrot.r = mrotMap.r;
	}
	if (mrot.g == -1)
	{
		mrot.g = mrotMap.g;
	}
	
	float metallic = mrot.r;
	float roughness = mrot.g;
	
	float dist = length(cameraPosition - position_FS);

	vec3 N = normalize(normal_FS);
	vec3 V = normalize((waterView.eyeAndCapture.w != 0.0 ? waterView.eyeAndCapture.xyz : cameraPosition) - position_FS);
	vec3 R = reflect(-V, N);
	
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedoColour,  metallic);
	
    vec3 Lo = vec3(0);

	// reflectance equation
	for(int i = 0; i < max_lights; ++i) 
    {
	    // calculate per-light radiance
        vec3 L = normalize(lights.position[i] - position_FS);
        vec3 H = normalize(V + L);
        float dist = length(lights.position[i] - position_FS);
        float attenuation = 1 / pow(dist, 2);
        vec3 radiance = lights.colour[i] * lights.intensity[i] * attenuation; 
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0), F0);
        
        vec3 kS = F;
        vec3 kD = vec3(1) - kS;
        kD *= 1 - metallic;
        
        vec3 numerator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0) * max(dot(N, L), 0);
        vec3 specular = numerator / max(denominator, 0.001);
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0);
        Lo += (kD * albedoColour / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03);

    vec3 colour = ambient * albedoColour + Lo;

    outColour = vec4(colour, 1.0);
    outBloom = vec4(0);
    if (waterView.eyeAndCapture.w != 0.0) outColour.a = gl_FragCoord.z;
}
