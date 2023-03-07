#version 430

layout (location = 0) out vec4 outColour;
layout (location = 1) out vec4 outPositionMetallic;
layout (location = 2) out vec4 outAlbedoRoughness;
layout (location = 3) out vec4 outNormal;
layout (location = 4) out vec4 outBloom;

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

uniform mat4 m_view;
layout(std140, binding = 0) uniform LightSpaceMatrices
{
	mat4 m_LightSpace[16];
};

uniform float time;

uniform vec3 sunPosition;
uniform vec3 sunColour;
uniform int sunIndex;

uniform int cascadeCount;
uniform float cascadePlaneDistances[16];
uniform float farPlane;
uniform sampler2DArray shadowMap;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);


float getShadow(vec3 fragPosWorldSpace, vec3 lightDir, vec3 normal)
{
	// select cascade layer
	vec4 fragPosViewSpace = m_view * vec4(fragPosWorldSpace, 1.0);
	float depthValue = abs(fragPosViewSpace.z);

	int layer = -1;
	for (int i = 0; i < cascadeCount; ++i)
	{
		if (depthValue < cascadePlaneDistances[i])
		{
			layer = i;
			break;
		}
	}
	if (layer == -1)
	{
		layer = cascadeCount;
	}

	vec4 fragPosLightSpace = m_LightSpace[layer] * vec4(fragPosWorldSpace, 1.0);

	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;;

	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	if (currentDepth > 1.0)
	{
		//return 0.0;
	}

	// calculate bias (based on depth map resolution and slope)
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	if (layer == cascadeCount)
	{
		bias *= 1 / max(farPlane * 0.5f, 0.005);
	}
	else
	{
		bias *= 1 / max(cascadePlaneDistances[layer] * 0.5f, 0.005);
	}

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(
				shadowMap,
				vec3(projCoords.xy + vec2(x, y) * texelSize,
					layer)
				).r;
			shadow += ((currentDepth - bias) > pcfDepth) ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	// keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
	if (projCoords.z > 1.0)
	{
		shadow = 0.0;
	}

	return shadow;
}

void main()
{
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

	const float reflectance = 0.5;
	const float transmittance = 0.0;
	float ior = reflectance + 1;
	
	vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition - position_FS);
    vec3 R = normalize(reflect(-V, N));
    vec3 Refr = normalize(refract(-V, N, 1.0 / ior));

    vec3 F0 = vec3(pow(reflectance, 2) * 0.16);
    F0 = mix(F0, albedoColour, metallic);

	vec3 sunDirection = normalize(-sunPosition);
	float shadow = getShadow(position_FS, sunDirection, N);

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
			attenuation = 0.0;
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

		if (sunIndex == index)
		{
			Lo += (kD * albedoColour / PI + specular) * radiance * NdotL * (1 - shadow);
		}
		else
		{
			Lo += (kD * albedoColour / PI + specular) * radiance * NdotL;
		}
    }

    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedoColour * occlusion;

    float lod = roughness * max_reflection_lod;
    vec3 prefilteredColour = textureLod(prefilterMap, R, lod).rgb;
    vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.01), roughness)).rg;
    vec3 specular = prefilteredColour * (F * envBRDF.x + envBRDF.y);


	vec3 irradianceR = texture(irradianceMap, -N).rgb;
	vec3 diffuseR = irradianceR * albedoColour * occlusion;

	vec3 prefilteredColourR = textureLod(prefilterMap, Refr, lod).rgb;
	vec3 specularR = prefilteredColourR * (F * envBRDF.x + envBRDF.y);



	vec3 ambient = (kD * diffuse * (1 - transmittance) + kD * diffuseR * transmittance + specular * (1 - transmittance) + specularR * transmittance);
    vec3 colour = ambient + Lo + clamp(albedoColour, vec3(0.05), vec3(1.0)) * max(emission, 0.0) * emissionFactor;

	if (isnan(colour.r))
	{
		colour = vec3(0.0);
	}

	colour *= max((1 - shadow), 0.2);

	outColour = vec4(colour, 1);
	outPositionMetallic = vec4(position_FS, metallic);
	outAlbedoRoughness = vec4(albedoColour, roughness);
	outNormal = vec4(N, 1);
	outBloom = vec4(clamp(albedoColour, vec3(0.05), vec3(1.0)) * max(emission, 0.0) * emissionFactor, 0);
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