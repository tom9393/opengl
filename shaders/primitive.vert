#version 460 core

// These locations MUST match the order of your BufferLayout
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
    vec3 u_CameraPosition;
};

layout(std430, binding = 0) readonly buffer InstanceData {
    mat4 u_InstanceTransforms[]; 
};

// Outputs to the Fragment Shader
out vec3 v_Normal;
out vec2 v_TexCoord;
out vec3 v_FragPos;

void main()
{
    mat4 a_InstanceTransform = u_InstanceTransforms[gl_InstanceID];
    // Calculate the position of the vertex in world space
    v_FragPos = vec3(a_InstanceTransform * vec4(a_Position, 1.0));
    
    // Calculate the normal vector. 
    // The inverse-transpose matrix ensures normals scale correctly if you scale your primitives non-uniformly!
    v_Normal = mat3(transpose(inverse(a_InstanceTransform))) * a_Normal; 
    
    v_TexCoord = a_TexCoord;

    // Final screen-space position
    gl_Position = u_ViewProjection * vec4(v_FragPos, 1.0);
}