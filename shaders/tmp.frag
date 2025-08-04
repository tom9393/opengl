#version 330 core

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

in vec2 v_TexCoord;
uniform Material material;

void main() {
    FragColor = texture(material.diffuse, v_TexCoord); // Testing purposes
}
