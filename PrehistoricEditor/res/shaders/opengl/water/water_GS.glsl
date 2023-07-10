#version 460

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 mapCoord_GS[];

out vec4 position_FS;
out vec2 mapCoord_FS;
out vec3 normal_FS;
out vec3 tangent_FS;

uniform sampler2D Dy;
uniform sampler2D Dx;
uniform sampler2D Dz;
uniform mat4 viewProjection;
uniform vec3 cameraPosition;
uniform vec2 wind;
uniform float displacementScale;
uniform float choppiness;
uniform float motion;
uniform int displacementRange;
uniform int highDetailRange;
uniform int tiling;

vec3 tangent;

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
	float dx, dy, dz;	
	vec4[] positions = { gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position };
	
	float dist = (distance(positions[0].xyz, cameraPosition)
			+ distance(positions[1].xyz, cameraPosition) 
			+ distance(positions[2].xyz, cameraPosition)) / 3;
			
	if(dist < displacementRange + 100)
	{
		if(dist < highDetailRange)
		{
			calcTangent();
		}
		
		for (int i = 0; i < gl_in.length(); ++i)
		{
			dy = texture(Dy, mapCoord_GS[i] + (wind * motion)).r * max(0,(-distance(gl_in[i].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * displacementScale;
			dx = texture(Dx, mapCoord_GS[i] + (wind * motion)).r * max(0,(-distance(gl_in[i].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
			dz = texture(Dz, mapCoord_GS[i] + (wind * motion)).r * max(0,(-distance(gl_in[i].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
	
			positions[i].y += dy;
			positions[i].x -= dx;
			positions[i].z -= dz;
		}
	}
	
	/*for (int i = 0; i < gl_in.length(); ++i)
	{
		gl_Position = viewProjection * positions[i];
		mapCoord_FS = mapCoord_GS[i];
		tangent_FS = tangent;
		EmitVertex();
	}*/
	
	gl_Position = viewProjection * positions[0];
	position_FS = positions[0];
	mapCoord_FS = mapCoord_GS[0];
	normal_FS = normalize(cross(positions[2].xyz - positions[0].xyz, positions[1].xyz - positions[0].xyz));
	tangent_FS = tangent;
	EmitVertex();
	
	gl_Position = viewProjection * positions[1];
	position_FS = positions[1];
	mapCoord_FS = mapCoord_GS[1];
	normal_FS = normalize(cross(positions[0].xyz - positions[1].xyz, positions[2].xyz - positions[1].xyz));
	tangent_FS = tangent;
	EmitVertex();
	
	gl_Position = viewProjection * positions[2];
	position_FS = positions[2];
	mapCoord_FS = mapCoord_GS[2];
	normal_FS = normalize(cross(positions[1].xyz - positions[2].xyz, positions[0].xyz - positions[2].xyz));
	tangent_FS = tangent;
	EmitVertex();
	
	EndPrimitive();
}