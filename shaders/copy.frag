#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Source;

void main()
{
    FragColor = texture(u_Source, TexCoords);
}
