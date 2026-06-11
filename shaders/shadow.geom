#version 460 core
// Passthrough that routes each shadow caster to its layer of the 2D shadow
// map array, so the whole array stays attached (and cleared) as one layered
// framebuffer instead of re-attaching a layer per caster.
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform int u_Layer;

void main()
{
    for (int i = 0; i < 3; ++i)
    {
        gl_Layer = u_Layer;
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
