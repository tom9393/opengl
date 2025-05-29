#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

#define NR_SPOT_LIGHTS 2

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;
out vec4 FragPosLightSpaceSpot[NR_SPOT_LIGHTS]; // Add this

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform mat4 spotLightSpaceMatrices[NR_SPOT_LIGHTS]; // Add this

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoord;

    FragPosLightSpace = lightSpaceMatrix * worldPos;

    for (int i = 0; i < NR_SPOT_LIGHTS; ++i) {
        FragPosLightSpaceSpot[i] = spotLightSpaceMatrices[i] * worldPos;
    }

    gl_Position = projection * view * worldPos;
}
