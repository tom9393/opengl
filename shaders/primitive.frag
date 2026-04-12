#version 460 core

layout(location = 0) out vec4 o_Color;

in vec3 v_Normal;
in vec2 v_TexCoord;
in vec3 v_FragPos;

// Uniforms you can pass from your Renderer / Material Component
uniform vec3 u_Color;      // E.g., vec3(0.8, 0.2, 0.2) for a red cube
uniform vec3 u_LightPos;   // World position of your scene's light
uniform vec3 u_ViewPos;    // World position of your camera

void main()
{
    // 1. Ambient Lighting (Base minimum light so the dark side isn't pitch black)
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * u_Color;
  	
    // 2. Diffuse Lighting (Directional brightness based on the light source)
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(u_LightPos - v_FragPos);
    
    // Calculate dot product between normal and light direction. Max prevents negative light.
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_Color;
    
    // 3. Specular Lighting (Optional: adds a slight shiny highlight)
    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0); // White specular highlight

    // Combine results
    vec3 result = ambient + diffuse + specular;
    o_Color = vec4(result, 1.0);
}