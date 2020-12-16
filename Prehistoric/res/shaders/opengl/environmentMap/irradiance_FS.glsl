#version 430

layout (location = 0) out vec4 out_colour;

layout (location = 0) in vec3 position_FS;

uniform samplerCube environmentMap;

const float PI = 3.1415926535897932384626433832795;

void main()
{		
    vec3 N = normalize(position_FS);
	
	vec3 irradiance = vec3(0.0);
	
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);

	const float TWO_PI = PI * 2.0;
	const float HALF_PI = PI * 0.5;

	float samplePhi = (2.0f * PI) / (180.0f * 4.0f);// 0.025;
	float sampleTheta = (0.5f * PI) / (64.0f * 4.0f);//0.025;
	int nrSamples = 0;
	
	for(float phi = 0.0; phi < TWO_PI; phi += samplePhi)
	{
		for(float theta = 0.0; theta < HALF_PI; theta += sampleTheta)
		{
			vec3 tempVec = cos(phi) * right + sin(phi) * up;
			vec3 sampleVec = cos(theta) * N + sin(theta) * tempVec;

			vec3 colour = texture(environmentMap, sampleVec).rgb;
			irradiance += colour * cos(theta) * sin(theta);
			nrSamples++;
	 	}
	}

	irradiance = PI * irradiance * (1.0 / float(nrSamples));
    out_colour = vec4(irradiance, 1.0);
}