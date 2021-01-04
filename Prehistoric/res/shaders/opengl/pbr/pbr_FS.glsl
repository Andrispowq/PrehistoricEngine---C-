#version 330

layout (location = 0) out vec4 positionMetallic;
layout (location = 1) out vec4 albedoRoughness;
layout (location = 2) out vec4 normalLit;
layout (location = 3) out vec4 emissionExtra;

in vec3 position_FS;
in vec2 texture_FS;
in vec3 normal_FS;
in vec3 tangent_FS;

struct Material
{
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D mrotMap;
	sampler2D emissionMap;
	
	vec3 colour;
	int usesNormalMap;
	vec4 mrot;
	vec3 emission;
};

uniform Material material;
uniform vec3 cameraPosition;
uniform int highDetailRange;

void main()
{
	vec3 albedoColour = material.colour;
	vec4 mrot = material.mrot;
	vec3 emission = material.emission;
	
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

	if (emission.r == -1)
	{
		emission = texture(material.emissionMap, texture_FS).rgb;
	}

	float dist = length(cameraPosition - position_FS);
	vec3 normal = normalize(normal_FS);	
	if(dist < highDetailRange && material.usesNormalMap == 1)
	{
		float attenuation = clamp(-dist / highDetailRange + 1.0, 0.0, 1.0);

		normal = 2.0 * texture(material.normalMap, texture_FS).rbg - 1.0;
		normal = normalize(normal);
		normal.xz *= attenuation;

		vec3 B = normalize(cross(tangent_FS, normal_FS));
		mat3 tbn = mat3(tangent_FS, normal_FS, B);

		normal = tbn * normal;
	}
	
	vec3 N = normalize(normal);

	positionMetallic = vec4(position_FS, mrot.r);
	albedoRoughness = vec4(albedoColour, mrot.g);
	normalLit = vec4(N, 1.0);
	emissionExtra = vec4(emission, mrot.b);
}