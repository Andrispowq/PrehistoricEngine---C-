#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) out vec4 outColour;

const int max_lights = 1000;
const float PI = 3.141592653589793;
const float emissionFactor = 3.0;

layout (location = 0) in vec3 position_FS;
layout (location = 1) in vec2 texture_FS;
layout (location = 2) in vec3 normal_FS;
layout (location = 3) in vec3 tangent_FS;

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

layout(set = 2, binding = 1, std140) uniform Material
{
	layout(offset = 00) vec4 mrot;
	layout(offset = 16) vec3 colour;
	layout(offset = 32) float emission;
	layout(offset = 48) int usesNormalMap;
} material;

layout (set = 1, binding = 0) uniform sampler2D albedoMap;
layout (set = 1, binding = 1) uniform sampler2D normalMap;
layout (set = 1, binding = 2) uniform sampler2D mrotMap;

layout (set = 0, binding = 3) uniform samplerCube irradianceMap;
layout (set = 0, binding = 4) uniform samplerCube prefilterMap;
layout (set = 0, binding = 5) uniform sampler2D brdfLUT;

layout(set = 0, binding = 6, std140) uniform Environment
{
	layout(offset = 0) float maxReflectionLOD;
};

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

void main()
{
	vec3 albedoColour = material.colour;
	vec4 mrot = material.mrot;
	float emission = max(material.emission, 0.0);
	
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
	if (mrot.b == -1)
	{
		mrot.b = mrotMap.b;
	}
	if (mrot.a == -1)
	{
		mrot.a = mrotMap.a;
	}
	
	float metallic = mrot.r;
	float roughness = mrot.g;
	float occlusion = mrot.b;
	
	float dist = length(position_FS - cameraPosition);	
	vec3 normal = normalize(normal_FS);
	if((dist < (highDetailRange - 50)) && (material.usesNormalMap == 1) && (dot(tangent_FS, vec3(0)) != 1))
	{
		float attenuation = clamp(-dist / (highDetailRange - 50) + 1.0, 0.0, 1.0);

		vec3 bumpNormal = 2.0 * texture(normalMap, texture_FS).rbg - 1.0;
		bumpNormal = normalize(bumpNormal);

		vec3 B = normalize(cross(tangent_FS, normal));
		mat3 tbn = mat3(tangent_FS, normal, B);

		bumpNormal.xz *= attenuation;
		normal = tbn * bumpNormal;
	}
	
	vec3 N = normalize(normal);
	vec3 V = normalize(cameraPosition - position_FS);
	vec3 R = reflect(-V, N);
	
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedoColour, metallic);
	
    vec3 Lo = vec3(0);
	
    // reflectance equation
	for(int i = 0; i < numberOfLights; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lights.position[i].xyz - position_FS);
        vec3 H = normalize(V + L);
        float dist = length(lights.position[i].xyz - position_FS);
        float attenuation = 1 / pow(dist, 2);
        vec3 radiance = lights.colour[i].rgb * lights.intensity[i].r * attenuation;
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0), F0);
        
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

	vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradiance = texture(irradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedoColour * occlusion;

	vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * maxReflectionLOD).rgb;
	vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.01), roughness)).rg;
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

	vec3 ambient = kD * diffuse + specular;
	
	// Match OpenGL: scalar emission strength tinted by the material albedo.
	vec3 colour = ambient + Lo + clamp(albedoColour, vec3(0.05), vec3(1.0)) * emission * emissionFactor;
	
	colour = 1.0 - exp(-colour * exposure);
	// CPU supplies gamma=1 for sRGB attachments; hardware encodes those once.
	if (gamma != 1.0)
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
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 2.0;

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
