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
    vec2 TexCoord;
    mat3 TBN;
};

uniform mat4 u_Transform;

layout (location = 0) out VertexOutput Output;

void main()
{
    // Calculate the TBN matrix for normal mapping
    vec3 T = normalize(vec3(u_Transform * vec4(a_Tangent,   0.0)));
    vec3 B = normalize(vec3(u_Transform * vec4(a_Bitangent, 0.0)));
    vec3 N = normalize(vec3(u_Transform * vec4(a_Normal,    0.0)));

    Output.TexCoord = a_TexCoord;
    Output.FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
    Output.TBN = mat3(T, B, N);

    gl_Position = u_ViewProjection * vec4(WorldPos, 1.0);
}
