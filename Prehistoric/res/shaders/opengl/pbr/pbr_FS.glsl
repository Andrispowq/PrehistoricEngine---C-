#version 330

layout (location = 0) out vec4 outColour;

const int max_lights = 10;
const float PI = 3.141592653589793;
const float emissionFactor = 3.0;

in vec3 position_FS;
in vec2 texture_FS;
in vec3 normal_FS;
in vec3 light_position[max_lights];
in vec3 camera_position;

in vec3 world_position_FS;
in vec3 world_normal_FS;
in vec3 world_tangent_FS;

struct Material
{
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D displacementMap;
	sampler2D metallicMap;
	sampler2D roughnessMap;
	sampler2D occlusionMap;
	sampler2D emissionMap;
	
	vec3 colour;
	int usesNormalMap;
	float heightScale;
	float metallic;
	float roughness;
	float ambientOcclusion;
	vec3 emission;
};

struct Light
{
	vec3 position;
	vec3 colour;
	vec3 intensity;
};

uniform Light lights[max_lights];
uniform Material material;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 cameraPosition;

uniform float gamma;
uniform float exposure;
uniform int highDetailRange;
uniform int numberOfLights;

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
	vec3 albedoColour = pow(getColour(material.albedoMap, material.colour, texture_FS), vec3(gamma));
	
	float metallic = getColour(material.metallicMap, material.metallic, texture_FS).r;
	float roughness = getColour(material.roughnessMap, material.roughness, texture_FS).r;
	float occlusion = getColour(material.occlusionMap, material.ambientOcclusion, texture_FS).r;
	
	vec3 emission = getColour(material.emissionMap, material.emission, texture_FS);

	float dist = length(camera_position - position_FS);
	vec3 normal = normalize(normal_FS);	

	vec3 Norm_world;
	if(dist < highDetailRange && material.usesNormalMap == 1)
	{
		float attenuation = clamp(-dist / highDetailRange + 1.0, 0.0, 1.0);
		
		normal = 2.0 * texture(material.normalMap, texture_FS).rbg - 1.0;
		normal = normalize(normal);
		normal.xz *= attenuation;

		vec3 B = cross(world_normal_FS, world_tangent_FS);
		mat3 tbn = mat3(world_tangent_FS, world_normal_FS, B);
		Norm_world = tbn * normal;
		Norm_world = normalize(Norm_world);
		Norm_world.xz *= attenuation;
	}
	
	vec3 N = normalize(normal);
	vec3 V = normalize(camera_position - position_FS);

	vec3 N_world = normalize(Norm_world);
	vec3 V_world = normalize(cameraPosition - world_position_FS);
	vec3 R = reflect(-V_world, Norm_world);
	
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
        vec3 radiance = lights[i].colour * lights[i].intensity * attenuation; 
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0), F0 );
        
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

	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradiance = texture(irradianceMap, N_world).rgb;
	vec3 diffuse = irradiance * albedoColour;
	
	const float MAX_REFLECTION_LOD = 4.0;
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilterMap, R, lodf).rgb;
	vec3 b = textureLod(prefilterMap, R, lodc).rgb;
	vec3 prefilteredColour = mix(a, b, lod - lodf);
	vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefilteredColour * (F * envBRDF.x + envBRDF.y);

	vec3 ambient = (kD * diffuse + specular) * occlusion;

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
    float r = roughness;
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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1 - F0) * pow(1.0 - cosTheta, 5.0);
} 