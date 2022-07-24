#version 460

layout(triangles) in;
layout(line_strip, max_vertices = 4) out;

in vec2 mapCoord_GS[];

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
uniform int tiling;

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
		dy = texture(Dy, mapCoord_GS[0] + (wind * motion)).r * max(0, (-distance(gl_in[0].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * displacementScale;
		dx = texture(Dx, mapCoord_GS[0] + (wind * motion)).r * max(0, (-distance(gl_in[0].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		dz = texture(Dz, mapCoord_GS[0] + (wind * motion)).r * max(0, (-distance(gl_in[0].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		
		position0.y += dy;
		position0.x -= dx;
		position0.z -= dz;
		
		dy = texture(Dy, mapCoord_GS[1] + (wind * motion)).r * max(0, (-distance(gl_in[1].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * displacementScale;
		dx = texture(Dx, mapCoord_GS[1] + (wind * motion)).r * max(0, (-distance(gl_in[1].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		dz = texture(Dz, mapCoord_GS[1] + (wind * motion)).r * max(0, (-distance(gl_in[1].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		
		position1.y += dy;
		position1.x -= dx;
		position1.z -= dz;

		dy = texture(Dy, mapCoord_GS[2] + (wind * motion)).r * max(0, (-distance(gl_in[2].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * displacementScale;
		dx = texture(Dx, mapCoord_GS[2] + (wind * motion)).r * max(0, (-distance(gl_in[2].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		dz = texture(Dz, mapCoord_GS[2] + (wind * motion)).r * max(0, (-distance(gl_in[2].gl_Position.xyz, cameraPosition) / displacementRange + 1)) * choppiness;
		
		position2.y += dy;
		position2.x -= dx;
		position2.z -= dz;
	}
	
    gl_Position = viewProjection * position0;
    EmitVertex();
	
	gl_Position = viewProjection * position1;
    EmitVertex();
	
	gl_Position = viewProjection * position2;
    EmitVertex();
	
    gl_Position = viewProjection * position0;
    EmitVertex();
	
    EndPrimitive();
}