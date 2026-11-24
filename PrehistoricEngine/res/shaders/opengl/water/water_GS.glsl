#version 430

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 mapCoord_GS[];

out vec4 position_FS;
out vec2 mapCoord_FS;
out vec3 normal_FS;

uniform mat4 viewProjection;

void main()
{
    vec3 n = cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz);
    for (int j = 0; j < 3; ++j)
    {
        int i = j % 3;
        position_FS = gl_in[i].gl_Position;
        mapCoord_FS = mapCoord_GS[i];
        normal_FS = n.y < 0.0 ? -n : n;
        gl_Position = viewProjection * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
