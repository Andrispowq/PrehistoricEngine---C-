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
uniform float scaleY;
uniform float choppiness;
uniform float motion;
uniform int displacementRange;
uniform int highDetailRange;

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
	
	vec4 position0 = gl_in[0].gl_Position;
	vec4 position1 = gl_in[1].gl_Position;
	vec4 position2 = gl_in[2].gl_Position;
	
	float dist = (distance(position0.xyz, cameraPosition)
			+ distance(position1.xyz, cameraPosition) 
			+ distance(position2.xyz, cameraPosition)) / 3;
			
	if(dist < displacementRange + 100)
	{
		if(dist < highDetailRange)
		{
			calcTangent();
		}
		
		dy = texture(Dy, mapCoord_GS[0] + (wind * motion)).r * max(0, (-distance(gl_in[0].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * scaleY;
		dx = texture(Dx, mapCoord_GS[0] + (wind * motion)).r * max(0, (-distance(gl_in[0].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		dz = texture(Dz, mapCoord_GS[0] + (wind * motion)).r * max(0, (-distance(gl_in[0].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		
		position0.y += dy;
		position0.x -= dx;
		position0.z -= dz;
		
		dy = texture(Dy, mapCoord_GS[1] + (wind * motion)).r * max(0, (-distance(gl_in[1].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * scaleY;
		dx = texture(Dx, mapCoord_GS[1] + (wind * motion)).r * max(0, (-distance(gl_in[1].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		dz = texture(Dz, mapCoord_GS[1] + (wind * motion)).r * max(0, (-distance(gl_in[1].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		
		position1.y += dy;
		position1.x -= dx;
		position1.z -= dz;
		
		dy = texture(Dy, mapCoord_GS[2] + (wind * motion)).r * max(0, (-distance(gl_in[2].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * scaleY;
		dx = texture(Dx, mapCoord_GS[2] + (wind * motion)).r * max(0, (-distance(gl_in[2].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		dz = texture(Dz, mapCoord_GS[2] + (wind * motion)).r * max(0, (-distance(gl_in[2].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		
		position2.y += dy;
		position2.x -= dx;
		position2.z -= dz;
	}
	
	gl_Position = viewProjection * position0;
	position_FS = position0;
	mapCoord_FS = mapCoord_GS[0];
	normal_FS = normalize(cross(position2.xyz - position0.xyz, position1.xyz - position0.xyz));
	tangent_FS = tangent;
	EmitVertex();
	
	gl_Position = viewProjection * position1;
	position_FS = position1;
	mapCoord_FS = mapCoord_GS[1];
	normal_FS = normalize(cross(position0.xyz - position1.xyz, position2.xyz - position1.xyz));
	tangent_FS = tangent;
	EmitVertex();
	
	gl_Position = viewProjection * position2;
	position_FS = position2;
	mapCoord_FS = mapCoord_GS[2];
	normal_FS = normalize(cross(position1.xyz - position2.xyz, position0.xyz - position2.xyz));
	tangent_FS = tangent;
	EmitVertex();
	
	EndPrimitive();
}