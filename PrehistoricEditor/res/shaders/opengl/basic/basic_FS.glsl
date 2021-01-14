#version 330

layout(location = 0) out vec4 positionMetallic;
layout(location = 1) out vec4 albedoRoughness;
layout(location = 2) out vec4 normalLit;
layout(location = 3) out vec4 emissionExtra;

in vec3 position_FS;
in vec2 texture_FS;
in vec3 normal_FS;

in vec3 view;

struct Material
{
	sampler2D albedoMap;
    sampler2D mrotMap;

    vec3 colour;
	vec4 mrot;
};

uniform Material material;
uniform vec3 cameraPosition;

void main()
{
	vec3 albedoColour = material.colour;
	vec4 mrot = material.mrot;
	
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
	
	vec3 N = normalize(normal_FS);

	positionMetallic = vec4(position_FS, mrot.r);
	albedoRoughness = vec4(albedoColour, mrot.g);
	normalLit = vec4(N, 1.0);
	emissionExtra = vec4(vec3(0.0), 1.0);
}