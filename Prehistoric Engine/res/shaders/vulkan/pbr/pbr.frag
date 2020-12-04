#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) out vec4 outColour;

const int max_lights = 10;
const float PI = 3.141592653589793;
const float emissionFactor = 3.0;

layout (location = 0) in vec3 position_FS;
layout (location = 1) in vec2 texture_FS;
layout (location = 2) in vec3 normal_FS;
layout (location = 3) in vec3 light_position[max_lights];
layout (location = 14) in vec3 camera_position;

layout(set = 0, binding = 0, std140) uniform Camera
{
	layout(offset = 0) mat4 m_view;
	layout(offset = 64) mat4 m_projection;
	layout(offset = 128) vec3 cameraPosition;
};

layout(set = 0, binding = 1, std140) uniform LightConditions
{
	layout(offset = 0) int highDetailRange;
	layout(offset = 4) int numberOfLights;
	layout(offset = 8) float gamma;
	layout(offset = 12) float exposure;
};

layout(set = 0, binding = 2, std140) uniform Lights
{
	layout(offset = max_lights * 0 * 16) vec4 position[max_lights];
	layout(offset = max_lights * 1 * 16) vec4 colour[max_lights];
	layout(offset = max_lights * 2 * 16) vec4 intensity[max_lights];
} lights;

layout(set = 1, binding = 0, std140) uniform Material
{
	layout(offset = 00) vec3 colour;
	layout(offset = 16) vec3 emission;
	layout(offset = 32) int usesNormalMap;
	layout(offset = 36) float heightScale;
	layout(offset = 40) float metallic;
	layout(offset = 44) float roughness;
	layout(offset = 48) float occlusion;
} material;

layout (set = 1, binding = 1) uniform sampler2D albedoMap;
layout (set = 1, binding = 2) uniform sampler2D displacementMap;
layout (set = 1, binding = 3) uniform sampler2D normalMap;
layout (set = 1, binding = 4) uniform sampler2D metallicMap;
layout (set = 1, binding = 5) uniform sampler2D roughnessMap;
layout (set = 1, binding = 6) uniform sampler2D occlusionMap;
layout (set = 1, binding = 7) uniform sampler2D emissionMap;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

vec3 getColour(sampler2D map, vec3 alternateValue, vec2 texCoords)
{
	if(alternateValue.r == -1)
		return texture(map, texCoords).rgb;
	else
		return alternateValue;
}

vec3 getColour(sampler2D map, float alternateValue, vec2 texCoords)
{
	if(alternateValue == -1)
		return texture(map, texCoords).rgb;
	else
		return vec3(alternateValue);
}

void main()
{
	vec3 albedoColour = pow(getColour(albedoMap, material.colour, texture_FS), vec3(gamma));
	
	float metallic = getColour(metallicMap, material.metallic, texture_FS).r;
	float roughness = getColour(roughnessMap, material.roughness, texture_FS).r;
	float occlusion = getColour(occlusionMap, material.occlusion, texture_FS).r;
	
	vec3 emission = getColour(emissionMap, material.emission, texture_FS);
	
	float dist = length(position_FS - cameraPosition);	
	vec3 normal = normalize(normal_FS);
	if(dist < highDetailRange && material.usesNormalMap == 1)
	{
		float attenuation = clamp(-dist / highDetailRange + 1.0, 0.0, 1.0);
		
		normal = 2.0 * texture(normalMap, texture_FS).rbg - 1.0;
		normal = normalize(normal);
		normal.xz *= attenuation;
	}
	
	vec3 N = normalize(normal);
	vec3 V = normalize(camera_position - position_FS);
	vec3 R = reflect(-V, N);
	
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedoColour, metallic);
	
    vec3 Lo = vec3(0);
	
    // reflectance equation
	for(int i = 0; i < numberOfLights; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(light_position[i] - position_FS);
        vec3 H = normalize(V + L);
        float dist = length(light_position[i] - position_FS);
        float attenuation = 1 / pow(dist, 2);
        vec3 radiance = lights.colour[i].rgb * lights.intensity[i].rgb * attenuation; 
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlickRoughness(max(dot(H, V), 0), F0, roughness);
        
        vec3 kS = F;
        vec3 kD = vec3(1) - kS;
        kD *= 1 - metallic;
        
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = numerator / max(denominator, 0.001);
        
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedoColour / PI + specular) * radiance * NdotL;
    }

	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradiance = vec3(0.23, 0.78, 0.88);//texture(irradianceMap, normal_FS).rgb;	
	vec3 diffuse = irradiance * albedoColour;
	
	/*const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
	vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;*/
	//vec3 specular = vec3(0.23, 0.78, 0.88);//prefilteredColor * (F * envBRDF.x + envBRDF.y);
	
	vec3 ambient = (kD * diffuse + vec3(0)) * occlusion;

	vec3 colour = ambient + Lo + max(emission * emissionFactor, 0.0);
	
	colour = 1.0 - exp(-colour * exposure);
	colour = pow(colour, vec3(1.0 / gamma));
	
	outColour = vec4(colour, 1);
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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1 - F0) * pow(1.0 - cosTheta, 5.0);
} 