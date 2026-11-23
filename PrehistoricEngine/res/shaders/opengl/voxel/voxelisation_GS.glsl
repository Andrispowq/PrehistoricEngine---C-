#version 460

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 position_GS[];
in vec3 normal_GS[];

out vec3 position_FS;
out vec3 normal_FS;

void main()
{
	const vec3 p1 = position_GS[1] - position_GS[0];
	const vec3 p2 = position_GS[2] - position_GS[0];
	const vec3 p = abs(cross(p1, p2)); 
	
	for(uint i = 0; i < 3; ++i)
	{
		position_FS = position_GS[i];
		normal_FS = normal_GS[i];
		gl_Position = vec4(position_FS, 1);
		
		/*if(p.z > p.x && p.z > p.y)
		{
			gl_Position = vec4(position_FS.x, position_FS.y, 0, 1);
		} 
		else if (p.x > p.y && p.x > p.z)
		{
			gl_Position = vec4(position_FS.y, position_FS.z, 0, 1);
		} 
		else 
		{
			gl_Position = vec4(position_FS.x, position_FS.z, 0, 1);
		}*/
		
		EmitVertex();
	}
	
    EndPrimitive();
}