#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

// Per-light cube face matrices, uploaded by ShadowPass3D through the frame pool.
layout(std140, binding = 2) uniform ShadowMatrices
{
    mat4 u_ShadowMatrices[6];
};

uniform int u_BaseLayer; // Specifies which light we are drawing

out vec4 FragPos;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = (u_BaseLayer * 6) + face;
        for(int i = 0; i < 3; ++i)
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = u_ShadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}