#version 460
#define M_PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outPositionMetallic;
layout(location = 2) out vec4 outAlbedoRoughness;
layout(location = 3) out vec4 outNormal;
layout(location = 4) out vec4 outBloom;

in vec4 position_FS;
in vec2 mapCoord_FS;
in vec3 normal_FS;
in vec3 tangent_FS;

uniform sampler2D Dy;
uniform sampler2D waterReflection;
uniform sampler2D waterRefraction;
uniform sampler2D normalMap;
uniform sampler2D dudvMap;
uniform mat4 worldMatrix;
uniform mat4 viewProjection;
uniform vec3 cameraPosition;
uniform vec3 waterColour;
uniform vec2 wind;
uniform float kReflection;
uniform float kRefraction;
uniform float distortion;
uniform float motion;
uniform float emission;
uniform float specularFactor;
uniform float specularAmplifier;
uniform float sightRangeFactor;
uniform float reflectionBlendFactor;
uniform float fresnelFactor;
uniform float capillarStrength;
uniform float capillarDownsampling;
uniform float dudvDownsampling;
uniform int isCameraUnderWater;
uniform int highDetailRange;
uniform int windowWidth;
uniform int windowHeight;
uniform int tiling;

uniform int numberOfTilesX;

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

layout(std430, binding = 0) readonly buffer LightBuffer
{
	PointLight data[];
} lightBuffer;

layout(std430, binding = 1) readonly buffer VisibleLightIndicesBuffer
{
	VisibleIndex data[];
} visibleLightIndicesBuffer;

const float Eta = 0.15;

float fresnelApproximated(vec3 normal, vec3 vertexToEye)
{
	vec3 halfDirection = normalize(normal + vertexToEye);
    float cosine = dot(halfDirection, vertexToEye);
	float fresnel = Eta + (1.0 - Eta) * pow(max(0.0, 1.0 - dot(vertexToEye, normal)), fresnelFactor);
	return clamp(pow(fresnel, 1.0),0.0,1.0);
}

float specularReflection(vec3 direction, vec3 normal, float specularFactor, float emissionFactor, vec3 vertexToEye)
{
	vec3 reflectionVector = normalize(reflect(direction, normal));
	float specular = max(0.0, dot(vertexToEye, reflectionVector));
	return pow(specular, specularFactor) * emissionFactor;
}

void main()
{
	vec3 bumpNormal;

	vec3 vertexToEye = normalize(cameraPosition - position_FS.xyz);
	float dist = length(cameraPosition - position_FS.xyz);
	
	vec2 waveMotion = wind * motion;
	
	vec3 normal = normal_FS;//texture(normalMap, mapCoord_FS + waveMotion).rbg;
	normal = normalize(normal);
	
	float fresnel = fresnelApproximated(normal.xyz, vertexToEye);
	
	if(dist < highDetailRange - 50)
	{
		float attenuation = clamp(-dist / (highDetailRange - 50) + 1, 0, 1);
		vec3 bitangent = normalize(cross(tangent_FS, normal_FS));
		mat3 tbn = mat3(bitangent, tangent_FS, normal_FS);
		bumpNormal = texture(normalMap, mapCoord_FS * capillarDownsampling + waveMotion).rgb;
		bumpNormal.z *= capillarStrength;
		bumpNormal.xy *= attenuation;
		bumpNormal = normalize(bumpNormal);
		normal = normalize(tbn * bumpNormal);
	}
	
	vec2 ndc = (position_FS.xy / position_FS.w) / 2 + 0.5;
	
	//projCoord
	vec3 dudvCoord = normalize((2 * texture(dudvMap, mapCoord_FS * dudvDownsampling + distortion).rbg) - 1);
	vec2 projCoord = vec2(gl_FragCoord.x / windowWidth, gl_FragCoord.y / windowHeight);
	
	//reflection
	vec2 reflecCoord = vec2(projCoord.x, projCoord.y) + dudvCoord.rb * kReflection;
	reflecCoord = clamp(reflecCoord, kReflection, 1 - kReflection);
	vec3 reflection = mix(texture(waterReflection, reflecCoord).rgb, waterColour, reflectionBlendFactor);
	reflection *= fresnel;
	
	//refraction
	vec2 refracCoord = projCoord.xy + dudvCoord.rb * kRefraction;
	refracCoord = clamp(refracCoord, kRefraction, 1 - kRefraction);
	
	vec3 refraction = texture(waterRefraction, refracCoord).rgb;
	
	//underwater -> only refraction
	if(isCameraUnderWater == 1)
	{
		reflection = vec3(0);
	} 
	else
	{
		refraction *= 1 - fresnel;
	}
	
	vec3 fragColour = reflection + refraction;
	
	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	uint index = uint(tileID.y * numberOfTilesX + tileID.x);
	
	vec3 Lo = vec3(0);
	/*uint offset = index * 1024;
    for (uint i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset + i].index != -1; i++)
    {
		uint lightIndex = visibleLightIndicesBuffer.data[offset + i].index;
		PointLight light = lightBuffer.data[lightIndex];
		
		vec3 lightPos = light.position.xyz;
		
		vec3 L = normalize(lightPos - position_FS.xyz);
		
		vec3 spec = vec3(specularReflection(L, normal, specularFactor, emission, vertexToEye));
        Lo += spec;
    }*/
	
	outColour = vec4(fragColour, 1);
	outPositionMetallic = vec4(position_FS.xyz, 0);
	outAlbedoRoughness = vec4(fragColour, 0);
	outNormal = vec4(normal, 1);
	outBloom = vec4(vec3(0.0), 0.0);
}