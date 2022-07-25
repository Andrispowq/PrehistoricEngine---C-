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
	vec4[] positions = { gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position };
	
	float dist = (distance(positions[0].xyz, cameraPosition)
			+ distance(positions[1].xyz, cameraPosition) 
			+ distance(positions[2].xyz, cameraPosition)) / 3;
	
	if(dist < displacementRange + 100)
	{		
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
	
    gl_Position = viewProjection * positions[0];
    EmitVertex();
	
	gl_Position = viewProjection * positions[1];
    EmitVertex();
	
	gl_Position = viewProjection * positions[2];
    EmitVertex();
	
    gl_Position = viewProjection * positions[0];
    EmitVertex();
	
    EndPrimitive();
}