#version 430

layout (location = 0) out vec4 out_colour;

layout (location = 0) in vec3 position_FS;

uniform samplerCube environmentMap;

const float PI = 3.141592653589793;

bool is_nan(vec3 vec, out int component)
{
	if (isnan(vec.x))
		component |= 0x1;
	if (isnan(vec.y))
		component |= 0x2;
	if (isnan(vec.z))
		component |= 0x4;

	return component != 0;
}

bool is_inf(vec3 vec, out int component)
{
	if (isinf(vec.x))
		component |= 0x8;
	if (isinf(vec.y))
		component |= 0x10;
	if (isinf(vec.z))
		component |= 0x20;

	return component != 0;
}

void main()
{		
    vec3 N = normalize(position_FS);
	
	vec3 irradiance = vec3(0.0);
	
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, N);
	up = cross(N, right);

	float sampleDelta = 0.025;
	int nrSamples = 0;
	
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			vec3 colour = texture(environmentMap, sampleVec).rgb;
			irradiance += colour * cos(theta) * sin(theta);
			nrSamples++;
	 	}
	}

	irradiance = PI * irradiance * (1.0 / float(nrSamples));
    out_colour = vec4(irradiance, 1.0);
}