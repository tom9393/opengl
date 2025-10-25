#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};

struct VertexOutput
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
};

uniform mat4 u_Transform;
uniform int u_EntityID;

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat int v_EntityID;

void main()
{
    vec4 WorldPos = u_Transform * vec4(a_Position, 1.0);
    Output.FragPos = vec3(WorldPos);
    Output.Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
    Output.TexCoord = a_TexCoord;

    gl_Position = u_ViewProjection * WorldPos;
    v_EntityID = u_EntityID;
}
