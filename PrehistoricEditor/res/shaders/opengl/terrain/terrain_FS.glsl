#version 430

layout(location = 0) out vec4 positionMetallic;
layout(location = 1) out vec4 albedoRoughness;
layout(location = 2) out vec4 normalLit;
layout(location = 3) out vec4 emissionExtra;

struct Material
{
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D mrotMap;

	float heightScale;
	float horizontalScale;
	vec4 mrot;
};

in vec3 position_FS;
in vec2 mapCoord_FS;
in vec3 tangent_FS;

uniform Material materials[3];

uniform sampler2D normalmap;
uniform sampler2D heightmap;
uniform sampler2D splatmap;

uniform vec3 cameraPosition;
uniform int highDetailRange;

void main()
{
	float dist = length(cameraPosition - position_FS);
	float height = position_FS.y;

	vec3 N = texture(normalmap, mapCoord_FS).rbg * 2.0 - 1.0;	
	vec4 blendValues = texture(splatmap, mapCoord_FS);	
	float[4] blendValueArray = float[](blendValues.r, blendValues.g, blendValues.b, blendValues.a);	
	if(dist < highDetailRange - 50)
	{
		float attenuation = clamp(-dist / (highDetailRange - 50) + 1, 0, 1);
		
		vec3 bitangent = normalize(cross(tangent_FS, N));
		mat3 tbn = mat3(tangent_FS, N, bitangent);
		
		vec3 bumpNormal;
		for(int i = 0; i < 3; i++)
		{
			bumpNormal += (2 * texture(materials[i].normalMap, mapCoord_FS * materials[i].horizontalScale).rbg - 1)
				* blendValueArray[i];
		}
		
		bumpNormal = normalize(bumpNormal);
		bumpNormal.xz *= attenuation;
		
		N = normalize(tbn * bumpNormal);
	}
	
	vec3 albedoColour = vec3(0);
	
	float metallic;
	float roughness;
	float occlusion;
	
	vec4 mrot;
	for(int i = 0; i < 3; i++)
	{
		vec2 texCoords = mapCoord_FS * materials[i].horizontalScale;
	
		albedoColour += pow(texture(materials[i].albedoMap, texCoords).rgb, vec3(2.2)) * blendValueArray[i];

		vec4 mrot_i = materials[i].mrot;
		vec4 mrotMap = texture(materials[i].mrotMap, texCoords);
		if (mrot_i.r == -1)
		{
			mrot_i.r = mrotMap.r;
		}
		if (mrot_i.g == -1)
		{
			mrot_i.g = mrotMap.g;
		}
		if (mrot_i.b == -1)
		{
			mrot_i.b = mrotMap.b;
		}
		if (mrot_i.a == -1)
		{
			mrot_i.a = mrotMap.a;
		}

		mrot += mrot_i * blendValueArray[i];
	}

	metallic = mrot.r;
	roughness = mrot.g;
	occlusion = mrot.b;

	positionMetallic = vec4(position_FS, metallic);
	albedoRoughness = vec4(albedoColour, roughness);
	normalLit = vec4(N, 0.9);
	emissionExtra = vec4(vec3(0.0), occlusion);
}