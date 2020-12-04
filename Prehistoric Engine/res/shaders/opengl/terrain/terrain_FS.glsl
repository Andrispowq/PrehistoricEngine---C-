#version 430

layout (location = 0) out vec4 outColour;

struct Material
{
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D displacementMap;
	sampler2D metallicMap;
	sampler2D roughnessMap;
	sampler2D occlusionMap;
	
	float heightScale;
	float horizontalScale;
	float metallic;
	float roughness;
	float occlusion;
};

struct Light
{
	vec3 position;
	vec3 colour;
	vec3 intensity;
};

in vec3 position_FS;
in vec2 mapCoord_FS;
in vec3 tangent_FS;

const int max_lights = 10;

uniform Material materials[3];
uniform Light lights[max_lights];
uniform sampler2D normalmap;
uniform sampler2D heightmap;
uniform sampler2D splatmap;
uniform vec3 cameraPosition;
uniform int highDetailRange;
uniform int numberOfLights;
uniform float gamma;
uniform float exposure;

//uniform samplerCube irradianceMap;
//uniform samplerCube prefilterMap;
//uniform sampler2D brdfLUT;

const float PI = 3.141592653589793;
const float emissionFactor = 3;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main()
{
	float dist = length(cameraPosition - position_FS);
	float height = position_FS.y;

	vec3 N = texture(normalmap, mapCoord_FS).rbg * 2.0 - 1.0;
	
	vec4 blendValues = texture(splatmap, mapCoord_FS);
	
	float[4] blendValueArray = float[](blendValues.r, blendValues.g, blendValues.b, blendValues.a);
	
	if(dist < highDetailRange - 50)
	{
		float attenuation = clamp(- dist / (highDetailRange - 50) + 1, 0, 1);
		
		vec3 bitangent = normalize(cross(tangent_FS, N));
		mat3 tbn = mat3(tangent_FS, N, bitangent);//mat3(bitangent, N, tangent_FS);
		
		vec3 bumpNormal;
		for(int i = 0; i < 3; i++)
		{
			bumpNormal += (2 * texture(materials[i].normalMap, mapCoord_FS * materials[i].horizontalScale).rbg - 1)
				* blendValueArray[i];
		}
		
		bumpNormal = normalize(bumpNormal);		
		bumpNormal.xz *= attenuation;
		
		N = normalize(tbn * bumpNormal);
	}
	
	vec3 albedoColour = vec3(0);
	
	float metallic;
	float roughness;
	float occlusion;
	
	vec3 V = normalize(cameraPosition - position_FS);
	vec3 R = reflect(V, N);
	
	for(int i = 0; i < 3; i++)
	{
		vec2 texCoords = mapCoord_FS * materials[i].horizontalScale;
	
		albedoColour += texture(materials[i].albedoMap, texCoords).rgb * blendValueArray[i];
	
		if(materials[i].metallic == -1)
			metallic += texture(materials[i].metallicMap, texCoords).r * blendValueArray[i];
		else
			metallic += materials[i].metallic * blendValueArray[i];
		
		if(materials[i].roughness == -1)
			roughness += texture(materials[i].roughnessMap, texCoords).r * blendValueArray[i];
		else
			roughness += materials[i].roughness * blendValueArray[i];
		
		if(materials[i].occlusion == -1)
			occlusion += texture(materials[i].occlusionMap, texCoords).r * blendValueArray[i];
		else
			occlusion += materials[i].occlusion * blendValueArray[i];
	}
	
	albedoColour = pow(albedoColour, vec3(gamma));
	
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedoColour, metallic);
	
    vec3 Lo = vec3(0);
	
	// reflectance equation
	for(int i = 0; i < numberOfLights; i++)
    {
        // calculate per-light radiance
        vec3 L = normalize(lights[i].position - position_FS);
        vec3 H = normalize(V + L);
        float dist = length(lights[i].position - position_FS);
        float attenuation = 1 / pow(dist, 2);
		vec3 radiance = lights[i].colour * lights[i].intensity * attenuation; 
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0), F0);
        
        vec3 kS = F;
        vec3 kD = vec3(1) - kS;
        kD *= 1 - metallic;
        
        vec3 numerator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0) * max(dot(N, L), 0);
        vec3 specular = numerator / max(denominator, 0.001);
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0);
		Lo += (kD * albedoColour / PI/* + specular*/) * radiance * NdotL; //TODO: in the terrain_FS this is a temporary solution, as the specular part causes some visual bugs
    }
	
	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);
	
	vec3 kS = F;
	vec3 kD = 1 - kS;
	kD *= 1 - metallic;
	
	vec3 irradiance = vec3(0.23, 0.78, 0.88);//texture(irradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedoColour;
	
	/*const float MAX_REFLECTION_LOD = 4;
	vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
	vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;*/
	//vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
	
	vec3 ambient = (kD * diffuse + 0) * occlusion;

	//vec3 ambient = vec3(0.03);
	
	vec3 colour = ambient * albedoColour + Lo;
	
	colour = 1.0 - exp(-colour * exposure);
	colour = pow(colour, vec3(1 / gamma));

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