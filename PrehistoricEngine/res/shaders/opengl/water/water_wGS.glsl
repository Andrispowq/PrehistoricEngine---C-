#version 430

layout(triangles) in;
layout(line_strip, max_vertices = 4) out;

in vec2 mapCoord_GS[];

uniform mat4 viewProjection;

void main()
{
    for (int j = 0; j < 4; ++j)
    {
        int i = j % 3;
        gl_Position = viewProjection * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
