#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform writeonly imageCube irradianceMap;

uniform samplerCube environmentMap;
uniform float resolution;

const float PI = 3.1415926535897932384626433832795;

const float TWO_PI = PI * 2.0;
const float HALF_PI = PI * 0.5;

const float samplePhi = 0.025;
const float sampleTheta = 0.025;
const float totalSamples = (TWO_PI / samplePhi) * (HALF_PI / sampleTheta);
const float invTotalSamples = 1.0 / totalSamples;

ivec3 texCoordToCube(vec3 texCoord, vec2 cubemapSize)
{
	vec3 abst = abs(texCoord);
	texCoord /= max(max(abst.x, abst.y), abst.z);

	float cubeFace;
	vec2 uvCoord;
	if (abst.x > abst.y && abst.x > abst.z)
	{
		// x major
		float negx = step(texCoord.x, 0.0);
		uvCoord = mix(vec2(-texCoord.z, -texCoord.y), vec2(texCoord.z, -texCoord.y), negx);
		cubeFace = negx;
	}
	else if (abst.y > abst.z)
	{
		// y major
		float negy = step(texCoord.y, 0.0);
		uvCoord = mix(vec2(texCoord.xz), vec2(texCoord.x, -texCoord.z), negy);
		cubeFace = 2.0 + negy;
	}
	else
	{
		// z major
		float negz = step(texCoord.z, 0.0);
		uvCoord = mix(vec2(texCoord.x, -texCoord.y), vec2(-texCoord.x, -texCoord.y), negz);
		cubeFace = 4.0 + negz;
	}

	uvCoord = (uvCoord + 1.0) * 0.5;
	uvCoord = uvCoord * cubemapSize;
	uvCoord = clamp(uvCoord, vec2(0.0), cubemapSize - vec2(1.0));

	return ivec3(ivec2(uvCoord), int(cubeFace));
}

void main()
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	vec2 texCoord = vec2(x) / resolution;
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
	
	vec3 irradiance = vec3(0.0);
	 
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, N);
	up = cross(N, right);

	for(float phi = 0.0; phi < TWO_PI; phi += samplePhi)
	{
		float sinPhi = sin(phi);
		float cosPhi = cos(phi);

		for(float theta = 0.0; theta < HALF_PI; theta += sampleTheta)
		{
			float sinTheta = sin(theta);
			float cosTheta = cos(theta);

			vec3 sphereCoord = vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
			vec3 sampleVec = sphereCoord.x * right + sphereCoord.y * up + sphereCoord.z * N;

			vec3 colour = texture(environmentMap, sampleVec).rgb;
			irradiance += colour * cosTheta * sinTheta;
	 	}
	}

	irradiance = PI * irradiance * invTotalSamples;
    imageStore(irradianceMap, ivec3(gl_GlobalInvocationID), vec4(irradiance, 1.0));
}