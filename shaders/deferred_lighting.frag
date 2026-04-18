#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gPBR;

uniform sampler2D u_DirSpotShadowMap;
uniform samplerCube u_PointShadowCubemap;
uniform float u_FarPlane;

struct Light {
    vec4 PositionType;
    vec4 ColorInner;
    vec4 DirectionOuter;
    mat4 LightSpaceMatrix;
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

// --- GGX / BRDF Functions ---
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / max(denom, 0.0000001); 
}

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

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// --- Shadow Calculation Functions ---
float ShadowCalculation2D(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0) return 0.0;
    
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    
    // Simple PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_DirSpotShadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(u_DirSpotShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}

float ShadowCalculationPoint(vec3 fragPos, vec3 lightPos) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    
    float closestDepth = texture(u_PointShadowCubemap, fragToLight).r;
    closestDepth *= u_FarPlane;
    
    float bias = 0.05; 
    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

void main()
{
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;
    vec3 N = texture(gNormal, TexCoords).rgb;
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    float metallic = texture(gPBR, TexCoords).r;
    float roughness = texture(gPBR, TexCoords).g;
    float ao = texture(gPBR, TexCoords).b;

    vec3 V = normalize(u_CameraPosition - WorldPos);
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    vec3 Lo = vec3(0.0);
    
    for(int i = 0; i < LightCount; i++)
    {
        Light light = Lights[i];
        int type = int(light.PositionType.w);
        vec3 lightColor = light.ColorInner.xyz;
        vec3 L;
        float attenuation = 1.0;
        float shadow = 0.0;

        if (type == 1) { // Directional
            L = normalize(-light.DirectionOuter.xyz);
            vec4 fragPosLightSpace = light.LightSpaceMatrix * vec4(WorldPos, 1.0);
            shadow = ShadowCalculation2D(fragPosLightSpace, N, L);
        }
        else if (type == 0) { // Point
            L = normalize(light.PositionType.xyz - WorldPos);
            float distance = length(light.PositionType.xyz - WorldPos);
            attenuation = 1.0 / (distance * distance);
            shadow = ShadowCalculationPoint(WorldPos, light.PositionType.xyz);
        }
        else if (type == 2) { // Cone (Spot)
            L = normalize(light.PositionType.xyz - WorldPos);
            float distance = length(light.PositionType.xyz - WorldPos);
            attenuation = 1.0 / (distance * distance);
            
            float theta = dot(L, normalize(-light.DirectionOuter.xyz));
            float epsilon = light.ColorInner.w - light.DirectionOuter.w;
            float intensity = clamp((theta - light.DirectionOuter.w) / epsilon, 0.0, 1.0);
            attenuation *= intensity;

            vec4 fragPosLightSpace = light.LightSpaceMatrix * vec4(WorldPos, 1.0);
            shadow = ShadowCalculation2D(fragPosLightSpace, N, L);
        }

        //shadow = 0.0; // For debugging

        vec3 radiance = lightColor * attenuation * (1.0 - shadow);

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

    // HDR Tonemapping
    color = color / (color + vec3(1.0)); 
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}