#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;

#define NR_SPOT_LIGHTS 2

layout (binding = 0, std430) readonly buffer MyBlock {
    mat4 modelMatrices[];
};

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoord;
out vec4 FragPosLightSpace;
out vec4 FragPosLightSpaceSpot[NR_SPOT_LIGHTS]; // Add this

// uniform mat4 u_Transform;
uniform mat4 u_ViewProjection;
uniform mat4 lightSpaceMatrix;
uniform mat4 spotLightSpaceMatrices[NR_SPOT_LIGHTS]; // Add this

void main()
{
    mat4 u_Transform = modelMatrices[gl_InstanceID];
    vec4 WorldPos = u_Transform * vec4(a_Position, 1.0);
    v_FragPos = vec3(WorldPos);
    v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
    v_TexCoord = a_TexCoord;

    FragPosLightSpace = lightSpaceMatrix * WorldPos;

    for (int i = 0; i < NR_SPOT_LIGHTS; ++i) {
        FragPosLightSpaceSpot[i] = spotLightSpaceMatrices[i] * WorldPos;
    }

    gl_Position = u_ViewProjection * WorldPos;
}
