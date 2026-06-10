#version 460 core

layout (location = 0) in vec2 a_Corner;   // unit quad corner in [-0.5, 0.5]
layout (location = 1) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
    vec3 u_CameraPosition;
};

struct ParticleInstance
{
    vec3 Position;
    float Size;
    vec4 Color;
};

layout (std430, binding = 0) readonly buffer ParticleData
{
    ParticleInstance u_Particles[];
};

uniform vec3 u_CameraRight;
uniform vec3 u_CameraUp;

out vec2 v_TexCoord;
out vec4 v_Color;

void main()
{
    ParticleInstance p = u_Particles[gl_InstanceID];

    vec3 worldPos = p.Position + (u_CameraRight * a_Corner.x + u_CameraUp * a_Corner.y) * p.Size;

    v_TexCoord = a_TexCoord;
    v_Color = p.Color;
    gl_Position = u_ViewProjection * vec4(worldPos, 1.0);
}
