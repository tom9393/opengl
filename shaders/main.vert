#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;

struct VertexOutput
{
    vec3 FragPos;
    vec2 TexCoord;
    mat3 TBN;
    vec4 FragPosLightSpace;
};

// Note: if you need bindings only in fragment shader you do not need to specify them here

layout (std140, binding = 0) uniform Camera // std140 = uniform buffer
{
    mat4 u_ViewProjection;
    vec3 u_CameraPosition;
};

layout(std430, binding = 0) readonly buffer InstanceData { // std430 = shader storage buffer
    mat4 u_InstanceTransforms[]; 
};

uniform mat4 u_LightSpaceMatrix;

layout (location = 0) out VertexOutput Output;

void main()
{
    mat4 a_InstanceTransform = u_InstanceTransforms[gl_InstanceID];
    vec3 WorldPos = vec3(a_InstanceTransform * vec4(a_Position, 1.0));
    // Calculate the TBN matrix for normal mapping
    vec3 T = normalize(vec3(a_InstanceTransform * vec4(a_Tangent,   0.0)));
    vec3 B = normalize(vec3(a_InstanceTransform * vec4(a_Bitangent, 0.0)));
    vec3 N = normalize(vec3(a_InstanceTransform * vec4(a_Normal,    0.0)));

    Output.FragPos = WorldPos;
    Output.TexCoord = a_TexCoord;
    Output.TBN = mat3(T, B, N);

    Output.FragPosLightSpace = u_LightSpaceMatrix * vec4(WorldPos, 1.0);

    gl_Position = u_ViewProjection * vec4(WorldPos, 1.0);
}
