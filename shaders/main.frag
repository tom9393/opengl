#version 460 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out int SelectedEntity;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct VertexOutput
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat int v_EntityID;

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform Material material;

void main()
{    
    FragColor = texture(material.diffuse, Input.TexCoord);
    SelectedEntity = v_EntityID;
}
