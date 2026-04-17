#version 460 core
layout (location = 0) in vec3 a_Position;

layout(std430, binding = 0) readonly buffer InstanceData {
    mat4 u_InstanceTransforms[];
};

void main()
{
    mat4 a_InstanceTransform = u_InstanceTransforms[gl_InstanceID];
    gl_Position = a_InstanceTransform * vec4(a_Position, 1.0);
}