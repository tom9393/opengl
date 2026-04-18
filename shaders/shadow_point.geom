#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

// Passed in from C++ SceneRenderer when doing Point Light Shadow pass
uniform mat4 u_ShadowMatrices[6];
uniform int u_BaseLayer; // Sent from SceneRenderer.cpp to specify which light we are drawing

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