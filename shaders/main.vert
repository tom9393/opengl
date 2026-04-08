#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;
layout (location = 5) in mat4 a_InstanceTransform;

struct PointLight {
    vec4 Position; 
    vec4 Color;
};

struct VertexOutput
{
    vec3 FragPos;
    vec2 TexCoord;
    mat3 TBN;
};

layout (std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
    vec3 u_CameraPosition;
};

#define MAX_LIGHTS 64

layout(std140, binding = 1) uniform LightData
{
    PointLight Lights[MAX_LIGHTS];
    int LightCount;
};

layout (location = 0) out VertexOutput Output;

void main()
{
    vec3 WorldPos = vec3(a_InstanceTransform * vec4(a_Position, 1.0));
    // Calculate the TBN matrix for normal mapping
    vec3 T = normalize(vec3(a_InstanceTransform * vec4(a_Tangent,   0.0)));
    vec3 B = normalize(vec3(a_InstanceTransform * vec4(a_Bitangent, 0.0)));
    vec3 N = normalize(vec3(a_InstanceTransform * vec4(a_Normal,    0.0)));

    Output.FragPos = WorldPos;
    Output.TexCoord = a_TexCoord;
    Output.TBN = mat3(T, B, N);

    gl_Position = u_ViewProjection * vec4(WorldPos, 1.0);
}
