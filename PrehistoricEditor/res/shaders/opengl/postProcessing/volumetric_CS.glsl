#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;
uniform sampler2D positionMetallicImage;
uniform sampler2D albedoRoughnessImage;
uniform sampler2D normalImage;
uniform sampler2DArray shadowMap;

uniform vec2 resolution;

uniform vec3 cameraPosition;

uniform vec3 sunPosition;
uniform vec3 sunColour;
uniform int sunIndex;

uniform int cascadeCount;
uniform float cascadePlaneDistances[16];
uniform float farPlane;

uniform mat4 m_view;
layout(std140, binding = 0) uniform LightSpaceMatrices
{
    mat4 m_LightSpace[16];
};

const float PI = 3.141592653589793;

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
	projCoords = projCoords * 0.5 + 0.5;

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
		bias *= 1 / (farPlane * 0.5f);
	}
	else
	{
		bias *= 1 / (cascadePlaneDistances[layer] * 0.5f);
	}

	// PCF
	float pcfDepth = texture(shadowMap, vec3(projCoords.xy, layer)).r;
	float shadow = ((currentDepth - bias) > pcfDepth) ? 1.0 : 0.0;

	// keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
	if (projCoords.z > 1.0)
	{
		//shadow = 0.0;
	}

	return shadow;
}

const float G_SCATTERING = 0.8;
const int NB_STEPS = 50;

vec3 RayleighScattering(float distance, float lightDotView, float intensity, float wavelength/*vec3 colour*/, float ior)
{
	vec3 result = vec3(intensity);
	result *= ((1 + pow(lightDotView, 2)) / (2 * distance));
	result *= pow(((2 * PI) / wavelength), 4);//colour;
	result *= pow(((ior * ior - 1) / (ior * ior + 2)), 2);
	return result;
}

float MieScattering(float lightDotView)
{
	float result = 1.0f - G_SCATTERING * G_SCATTERING;
	result /= (4.0f * PI * pow(1.0f + G_SCATTERING * G_SCATTERING - (2.0f * G_SCATTERING) * lightDotView, 1.5f));
	return result;
}

vec3 ComputeScattering(float lightDotView, float distance)
{
	float mie = MieScattering(lightDotView);
	vec3 rayleigh = RayleighScattering(distance, lightDotView, 1000.0, 700/*vec3(0.9, 0.6, 0.3)*/, 1.1);

	return vec3(mie) + rayleigh;
}

vec3 getFog(vec3 worldPosition, vec3 lightDir, vec3 lightColour, vec3 normal)
{
	vec3 startPosition = worldPosition;
	vec3 endPosition = cameraPosition;

	vec3 rayVector = endPosition - startPosition;
	float rayLength = length(rayVector);
	float steps = float(NB_STEPS);
	vec3 rayDirection = rayVector / rayLength;
	float stepLength = rayLength / steps;
	vec3 step = rayDirection * stepLength;

	vec3 currentPosition = startPosition;
	vec3 accumFog = vec3(0.0);
	for (int i = 0; i < NB_STEPS; i++)
	{
		float shadow = getShadow(currentPosition, lightDir, normal);
		if (shadow == 0.0)
		{
			accumFog += ComputeScattering(dot(rayDirection, lightDir), length(sunPosition - currentPosition)) * /*stepLength */ lightColour;
		}

		currentPosition += step;
	}

	return accumFog;
}


void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = gl_GlobalInvocationID.xy / resolution;

    vec3 sceneColour = texture(scene, texCoord).rgb;
    vec4 positionMetallic = texture(positionMetallicImage, texCoord);
    vec4 albedoRoughness = texture(albedoRoughnessImage, texCoord);
    vec3 normal = texture(normalImage, texCoord).rgb;

	vec3 accumFog = getFog(positionMetallic.rgb, -normalize(sunPosition), vec3(0.9, 0.6, 0.3), normal);
	vec3 total = sceneColour;//mix(sceneColour, accumFog, 0.8);// + accumFog;
    imageStore(outColour, x, vec4(total, 1.0));
}