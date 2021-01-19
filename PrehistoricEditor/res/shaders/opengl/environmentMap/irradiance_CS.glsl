#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform writeonly imageCube irradianceMap;

uniform samplerCube environmentMap;
uniform float resolution;
uniform float resolution_irradianceMap;

const float PI = 3.1415926535897932384626433832795;

float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint N);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);

void main()
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	vec2 texCoord = vec2(x) / resolution_irradianceMap;
	texCoord.y = 1.0 - texCoord.y;
	texCoord *= 2.0;
	texCoord -= 1.0;

	uint face = gl_GlobalInvocationID.z;

	vec3 sampleDir = vec3(0.0);
	switch (face)
	{
	case 0: //pos x
		sampleDir = vec3(1.0, texCoord.y, texCoord.x);
		break;
	case 1: //neg x
		sampleDir = vec3(-1.0, texCoord.y, -texCoord.x);
		break;
	case 2: //pos y
		sampleDir = vec3(texCoord.x, 1.0, texCoord.y);
		break;
	case 3: //neg y
		sampleDir = vec3(texCoord.x, -1.0, -texCoord.y);
		break;
	case 4: //pos z
		sampleDir = vec3(texCoord.x, texCoord.y, -1.0);
		break;
	case 5: //neg z
		sampleDir = vec3(-texCoord.x, texCoord.y, 1.0);
		break;
	}

	vec3 N = normalize(vec3(sampleDir.xy, -sampleDir.z));
	vec3 R = N;
	vec3 V = R;

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, N);
	up = cross(N, right);

	vec3 irradiance = vec3(0.0);
	const uint SAMPLE_COUNT = 16384u;
	const float roughness = 1.0;
	float totalWeight = 0.0;

	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi, N, 1.0);

		float NdotH = max(dot(N, H), 0.0);
		float D = 1.0 / PI;
		float pdf = (D * NdotH / (4.0)) + 0.0001;

		float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
		float saTexel = 4.0 * PI / (6.0 * resolution * resolution);

		float mipLevel = 0.5 * log2(saSample / saTexel);

		irradiance += textureLod(environmentMap, H, mipLevel).rgb * NdotH;
		totalWeight += NdotH;
	}

	irradiance = PI * irradiance / totalWeight;
	imageStore(irradianceMap, ivec3(gl_GlobalInvocationID), vec4(irradiance, 1.0));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}