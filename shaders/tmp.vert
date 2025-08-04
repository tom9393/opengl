#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;

in mat4 u_Transform;
in mat4 u_ViewProjection;

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main()
{
    vec4 WorldPos = u_Transform * vec4(a_Position, 1.0);
    v_FragPos = vec3(WorldPos);
    v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
    v_TexCoord = a_TexCoord;

    gl_Position = u_ViewProjection * WorldPos;
}
