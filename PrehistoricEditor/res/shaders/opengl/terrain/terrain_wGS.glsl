#version 430

layout (triangles) in;
layout (line_strip, max_vertices = 4) out;

struct Material
{
	sampler2D mrotMap;
	
	float heightScale;
	float horizontalScale;
};

in vec2 mapCoord_GS[];

out vec3 position_FS;

uniform Material materials[3];
uniform sampler2D splatmap;
uniform mat4 viewProjection;
uniform vec3 cameraPosition;
uniform int highDetailRange;

vec3 tangent;
vec3 displacement[3];

void calcTangent()
{
	vec3 v0 = gl_in[0].gl_Position.xyz;
	vec3 v1 = gl_in[1].gl_Position.xyz;
	vec3 v2 = gl_in[2].gl_Position.xyz;
	
	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;
	
	vec2 uv0 = mapCoord_GS[0];
	vec2 uv1 = mapCoord_GS[1];
	vec2 uv2 = mapCoord_GS[2];
	
	vec2 deltaUV1 = uv1 - uv0;
	vec2 deltaUV2 = uv2 - uv0;
	
	float r = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
	
	tangent = normalize((e1 * deltaUV2.y - e2 * deltaUV1.y) * r);
}

void main()
{
	for(int i = 0; i < 3; ++i)
		displacement[i] = vec3(0);

	float dist = (distance(gl_in[0].gl_Position.xyz, cameraPosition)
				+ distance(gl_in[1].gl_Position.xyz, cameraPosition)
				+ distance(gl_in[2].gl_Position.xyz, cameraPosition)) / 3;
				
	if(dist < highDetailRange)
	{
		calcTangent();
		
		for(int i = 0; i < gl_in.length(); i++)
		{
			displacement[i] = vec3(0, 1, 0);
			
			float height = gl_in[i].gl_Position.y;
			
			vec4 blendValues = texture(splatmap, mapCoord_GS[i]);
			
			float[4] blendValueArray = float[](blendValues.r, blendValues.g, blendValues.b, blendValues.a);
			
			float scale = 0;
			for(int j = 0; j < 3; j++)
			{
				scale += texture(materials[j].mrotMap, mapCoord_GS[i]
							* materials[j].horizontalScale).a
							* materials[j].heightScale
							* blendValueArray[j];
			}
			
			float attenuation = clamp(- distance(gl_in[i].gl_Position.xyz, cameraPosition) / (highDetailRange - 50) + 1, 0.0, 1.0);
			scale *= attenuation;
			
			displacement[i] *= scale;
		}
	}

	for(int i = 0; i < gl_in.length(); ++i)
	{
		vec4 worldPos = gl_in[i].gl_Position + vec4(displacement[i], 0);
		gl_Position = viewProjection * worldPos;
		position_FS = worldPos.xyz;

		EmitVertex();
	}
	
	vec4 worldPos = gl_in[0].gl_Position + vec4(displacement[0], 0);
	gl_Position = viewProjection * worldPos;
	position_FS = worldPos.xyz;

	EmitVertex();
	
	EndPrimitive();
}