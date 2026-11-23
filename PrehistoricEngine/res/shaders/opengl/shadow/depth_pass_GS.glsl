#version 430

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;
    
layout (std140, binding = 0) uniform LightSpaceMatrices
{
    mat4 m_LightSpace[16];
};

void main()
{
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = m_LightSpace[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}