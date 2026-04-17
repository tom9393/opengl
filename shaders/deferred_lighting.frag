#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords; // From fullscreen quad vertex shader

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gPBR;
uniform sampler2D u_ShadowMap;

struct Light {
    vec4 PositionType;
    vec4 ColorInner;
    vec4 DirectionOuter;
};

layout (std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
    vec3 u_CameraPosition;
};

#define MAX_LIGHTS 64

layout(std140, binding = 1) uniform LightData
{
    Light Lights[MAX_LIGHTS];
    int LightCount;
};

const float PI = 3.14159265359;

// 1. Normal Distribution Function (GGX)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001); // Prevent divide by zero
}

// 2. Geometry Function (Smith)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// 3. Fresnel Equation (Schlick)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Basic shadow calculation function
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perform perspective divide and transform to [0,1] range
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // Get closest depth value from light's perspective
    float closestDepth = texture(u_ShadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    
    // Simple bias to prevent shadow acne
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    
    return shadow;
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perform perspective divide and transform to [0,1] range
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // Get closest depth value from light's perspective
    float closestDepth = texture(u_ShadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    
    // Simple bias to prevent shadow acne
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    
    return shadow;
}

void main()
{
    // 1. Unpack G-Buffer
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;
    vec3 N = texture(gNormal, TexCoords).rgb;
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    float metallic = texture(gPBR, TexCoords).r;
    float roughness = texture(gPBR, TexCoords).g;
    float ao = texture(gPBR, TexCoords).b;

    // 2. Lighting Calculations (Paste your existing lighting loop here!)
    vec3 V = normalize(u_CameraPosition - WorldPos);
    vec3 Lo = vec3(0.0);
    
    // Calculate shadows (assuming light 0 is your directional light)
    float shadow = 0.0;
    
    for(int i = 0; i < LightCount; i++)
    {
        Light light = Lights[i];
        
        int type = int(light.PositionType.w);
        vec3 lightColor = light.ColorInner.xyz;
        vec3 L;
        float attenuation = 1.0;
        
        // If it's a directional light, calculate shadows
        if (type == 1) {
            vec4 fragPosLightSpace = u_LightSpaceMatrix * vec4(WorldPos, 1.0);
            shadow = ShadowCalculation(fragPosLightSpace, N, L);
        }
        
        // Apply shadow to radiance
        vec3 radiance = lightColor * attenuation * (1.0 - shadow);
        
        // --- PBR BRDF Evaluation (Remains Unchanged) ---
        vec3 H = normalize(V + L);
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);        
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
    
    // Add Emission (scaled by albedo tint or kept pure white based on your preference)
    color += pbr.albedoTint.rgb * pbr.emission; 

    // HDR Tonemapping and Gamma Correction
    color = color / (color + vec3(1.0)); 
    color = pow(color, vec3(1.0/2.2));  
    
    FragColor = vec4(color, 1.0);
}