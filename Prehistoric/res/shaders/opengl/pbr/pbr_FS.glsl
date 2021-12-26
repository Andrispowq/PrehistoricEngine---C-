#version 430

layout (location = 0) out vec4 outColour;

in vec3 position_FS;
in vec2 texture_FS;
in vec3 normal_FS;
in vec3 tangent_FS;

struct Material
{
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D mrotMap;
	
	vec3 colour;
	int usesNormalMap;
	vec4 mrot;
	float emission;
};

struct PointLight 
{
	vec4 position;
	vec4 colour;
	vec4 intensity_radius;
};

struct VisibleIndex
{
	int index;
};

layout (std430, binding = 0) readonly buffer LightBuffer
{
	PointLight data[];
} lightBuffer;

layout (std430, binding = 1) readonly buffer VisibleLightIndicesBuffer 
{
	VisibleIndex data[];
} visibleLightIndicesBuffer;

const float PI = 3.141592653589793;
const float emissionFactor = 3;

uniform Material material;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 cameraPosition;
uniform int highDetailRange;
uniform int numberOfTilesX;
uniform float max_reflection_lod;
uniform float threshold;

in vec4 shadowCoord;
uniform sampler2D shadowTex;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

void main()
{
	const int pcfCount = 2;
	const float totalTexels = (pcfCount * 2.0 + 1.0) * (pcfCount * 2.0 + 1.0);

	float mapSize = 4096.0;
	float texelSize = 1.0 / mapSize;
	float total = 0.0;

	for (int x = -pcfCount; x <= pcfCount; x++) 
	{
		for (int y = -pcfCount; y <= pcfCount; y++)
		{
			float objectNearestLight = texture(shadowTex, shadowCoord.xy + vec2(x, y) * texelSize).r;
			if (shadowCoord.z > objectNearestLight + 0.002) 
			{
				total += 1.0;
			}
		}
	}

	total /= totalTexels;
	float lightFactor = 1.0 - (total * shadowCoord.w);


	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	uint index = uint(tileID.y * numberOfTilesX + tileID.x);

	vec3 albedoColour = material.colour;
	vec4 mrot = material.mrot;
	float emission = material.emission.r;
	
	if (albedoColour.r == -1)
	{
		albedoColour = pow(texture(material.albedoMap, texture_FS).rgb, vec3(2.2));
	}
	
	vec4 mrotMap = texture(material.mrotMap, texture_FS);
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

	float dist = length(cameraPosition - position_FS);
	vec3 normal = normalize(normal_FS);	
	if((dist < (highDetailRange - 50)) && (material.usesNormalMap == 1) && (dot(tangent_FS, vec3(0)) != 1))
	{
		float attenuation = clamp(-dist / (highDetailRange - 50) + 1.0, 0.0, 1.0);

		vec3 bumpNormal = 2.0 * texture(material.normalMap, texture_FS).rbg - 1.0;
		bumpNormal = normalize(bumpNormal);

		vec3 B = normalize(cross(tangent_FS, normal));
		mat3 tbn = mat3(tangent_FS, normal, B);

		bumpNormal.xz *= attenuation;
		normal = tbn * bumpNormal;
	}
	
	vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition - position_FS);
    vec3 R = normalize(reflect(-V, N));

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedoColour, metallic);

    vec3 Lo = vec3(0);
	uint offset = index * 1024;
    for (uint i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset + i].index != -1; i++)
    {
		int lightIndex = visibleLightIndicesBuffer.data[offset + i].index;
		PointLight light = lightBuffer.data[lightIndex];
		
		vec3 lightPos = light.position.xyz;
		
        vec3 L = normalize(lightPos - position_FS);
        vec3 H = normalize(V + L);
		float dist = length(lightPos - position_FS);
		float attenuation = 1.0 / pow(dist, 2);

		if (dist > light.intensity_radius.g)
		{
			attenuation = 0;
		}

        vec3 radiance = light.colour.rgb * light.intensity_radius.r * attenuation;

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
        Lo += (kD * albedoColour / PI + specular) * radiance * NdotL;
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
    vec3 colour = ambient + Lo + (clamp(albedoColour, vec3(0.05), vec3(1.0)) * max(emission, 0.0) * emissionFactor);

	if (isnan(colour.r))
	{
		colour = vec3(0.0);
	}

	outColour = vec4(colour * lightFactor, 1);
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