#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform writeonly imageCube irradianceMap;

uniform samplerCube environmentMap;
uniform float resolution;

const float PI = 3.1415926535897932384626433832795;

const float TWO_PI = PI * 2.0;
const float HALF_PI = PI * 0.5;

const float sampleDelta = 0.025 / 4;
const float totalSamples = (TWO_PI / sampleDelta) * (HALF_PI / sampleDelta);
const float invTotalSamples = 1.0 / totalSamples;

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
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);

	for(float phi = 0.0; phi < TWO_PI; phi += sampleDelta)
	{
		float sinPhi = sin(phi);
		float cosPhi = cos(phi);

		for(float theta = 0.0; theta < HALF_PI; theta += sampleDelta)
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