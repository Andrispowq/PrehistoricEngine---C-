#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec4 outColour;

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

layout (set = 0, binding = 2, std140) uniform LightConditions
{
	layout (offset = 0) float exposure;
	layout (offset = 4) float gamma;
};

layout (set = 1, binding = 1, std140) uniform Materials
{
    layout (offset = 00) vec3 colour;	
	layout (offset = 16) vec4 mrot;
} material;

layout(set = 1, binding = 2) uniform sampler2D albedoMap;
layout(set = 1, binding = 3) uniform sampler2D mrotMap;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

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
	vec3 V = normalize(cameraPosition - position_FS);
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

    colour = 1.0 - exp(-colour * exposure);
	colour = pow(colour, vec3(1.0 / gamma));

    outColour = vec4(colour, 1.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
	
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1 - F0) * pow(1.0 - cosTheta, 5.0);
} 