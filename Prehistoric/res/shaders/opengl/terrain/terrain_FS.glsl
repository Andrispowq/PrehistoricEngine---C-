#version 430

layout (location = 0) out vec4 outColour;

struct Material
{
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D mrotMap;

	float heightScale;
	float horizontalScale;
	vec4 mrot;
};

struct PointLight 
{
	vec4 colour;
	vec4 position;
	vec4 paddingAndRadius;
};

struct VisibleIndex 
{
	int index;
};

layout(std430, binding = 0) readonly buffer LightBuffer 
{
	PointLight data[];
} lightBuffer;

layout(std430, binding = 1) readonly buffer VisibleLightIndicesBuffer 
{
	VisibleIndex data[];
} visibleLightIndicesBuffer;

in vec3 position_FS;
in vec2 mapCoord_FS;
in vec3 tangent_FS;

uniform Material materials[3];

uniform sampler2D normalmap;
uniform sampler2D heightmap;
uniform sampler2D splatmap;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 cameraPosition;
uniform int highDetailRange;
uniform int numberOfTilesX;
uniform int max_reflection_lod;

const float PI = 3.141592653589793;
const float emissionFactor = 3;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

float attenuate(vec3 lightDirection, float radius) 
{
	float cutoff = 0.5;
	float attenuation = dot(lightDirection, lightDirection) / (100.0 * radius);
	attenuation = 1.0 / (attenuation * 15.0 + 1.0);
	attenuation = (attenuation - cutoff) / (1.0 - cutoff);

	return clamp(attenuation, 0.0, 1.0);
}

void main()
{
	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	uint index = uint(tileID.y * numberOfTilesX + tileID.x);

	float dist = length(cameraPosition - position_FS);
	float height = position_FS.y;

	vec3 N = texture(normalmap, mapCoord_FS).rbg * 2.0 - 1.0;	
	vec4 blendValues = texture(splatmap, mapCoord_FS);	
	float[4] blendValueArray = float[](blendValues.r, blendValues.g, blendValues.b, blendValues.a);	
	if(dist < highDetailRange - 50)
	{
		float attenuation = clamp(-dist / (highDetailRange - 50) + 1, 0, 1);
		
		vec3 bitangent = normalize(cross(tangent_FS, N));
		mat3 tbn = mat3(tangent_FS, N, bitangent);
		
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
	
	vec4 mrot;
	for(int i = 0; i < 3; i++)
	{
		vec2 texCoords = mapCoord_FS * materials[i].horizontalScale;
	
		albedoColour += pow(texture(materials[i].albedoMap, texCoords).rgb, vec3(2.2)) * blendValueArray[i];

		vec4 mrot_i = materials[i].mrot;
		vec4 mrotMap = texture(materials[i].mrotMap, texCoords);
		if (mrot_i.r == -1)
		{
			mrot_i.r = mrotMap.r;
		}
		if (mrot_i.g == -1)
		{
			mrot_i.g = mrotMap.g;
		}
		if (mrot_i.b == -1)
		{
			mrot_i.b = mrotMap.b;
		}
		if (mrot_i.a == -1)
		{
			mrot_i.a = mrotMap.a;
		}

		mrot += mrot_i * blendValueArray[i];
	}

	metallic = mrot.r;
	roughness = mrot.g;
	occlusion = mrot.b;

    vec3 V = normalize(cameraPosition - position_FS);
    vec3 R = normalize(reflect(-V, N));

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedoColour, metallic);

    vec3 Lo = vec3(0);
	uint offset = index * 1024;
    for (uint i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset + i].index != -1; i++)
    {
		uint lightIndex = visibleLightIndicesBuffer.data[offset + i].index;
		PointLight light = lightBuffer.data[lightIndex];
		
		vec3 lightPos = light.position.xyz;
		
        vec3 L = normalize(lightPos - position_FS);
        vec3 H = normalize(V + L);
        float attenuation = attenuate(L, light.paddingAndRadius.w);
        vec3 radiance = light.colour.rgb * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1) - kS;
        kD *= 1 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0) * max(dot(N, L), 0);
        vec3 specular = numerator / max(denominator, 0.001);

        float NdotL = max(dot(N, L), 0);
        Lo += (kD * albedoColour / PI) * radiance * NdotL;
    }

    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedoColour;

    float lod = roughness * max_reflection_lod;
    vec3 prefilteredColour = textureLod(prefilterMap, R, lod).rgb;
    vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.01), roughness)).rg;
    vec3 specular = prefilteredColour * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuse + specular) * occlusion;
    vec3 colour = ambient + Lo;

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
    float k = (r * r) / 2.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

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