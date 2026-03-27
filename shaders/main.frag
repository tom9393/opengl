#version 460 core

layout (location = 0) out vec4 FragColor;

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

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;
in vec4 v_Color;

uniform Material material;

void main()
{    
    FragColor = texture(material.diffuse, Input.TexCoord);
//    FragColor = Input.Color;
//    SelectedEntity = v_EntityID;
}
