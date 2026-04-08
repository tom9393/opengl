#version 460 core

layout (location = 0) out vec4 FragColor;

struct PBR {
    sampler2D albedoMap;
    bool useAlbedoMap;
    vec4 albedoTint;

    sampler2D normalMap;
    bool useNormalMap;

    sampler2D metallicMap;
    bool useMetallicMap;
    float metallicMultiplier;

    sampler2D roughnessMap;
    bool useRoughnessMap;
    float roughnessMultiplier;

    sampler2D aoMap;
    bool useAOMap;
    
    float emission;
};

struct VertexOutput
{
    vec3 FragPos;
    vec2 TexCoord;
    mat3 TBN;
};

struct PointLight {
    vec4 Position; 
    vec4 Color;
};

layout (location = 0) in VertexOutput Input;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
    vec3 u_CameraPosition;
};

#define MAX_LIGHTS 64

layout(std140, binding = 1) uniform LightData
{
    PointLight Lights[MAX_LIGHTS];
    int LightCount;
};

const float PI = 3.14159265359;

uniform PBR pbr;

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

void main()
{
    vec2 TexCoords = Input.TexCoord;
    mat3 TBN = Input.TBN;
    vec3 WorldPos = Input.FragPos;

    // --- PROPERTY EXTRACTION WITH FALLBACKS ---

    // 1. Albedo
    vec3 albedo = pbr.albedoTint.rgb;
    if (pbr.useAlbedoMap) {
        vec3 texColor = pow(texture(pbr.albedoMap, TexCoords).rgb, vec3(2.2));
        albedo *= texColor;
    }

    // 2. Normal
    vec3 N;
    if (pbr.useNormalMap) {
        vec3 normalMap = texture(pbr.normalMap, TexCoords).rgb;
        normalMap = normalMap * 2.0 - 1.0; 
        N = normalize(TBN * normalMap);
    } else {
        // If no normal map, use the vertex normal.
        // TBN[2] represents the Z axis of the matrix, which is the original Normal vector!
        N = normalize(TBN[2]);
    }

    // 3. Metallic
    float metallic = pbr.metallicMultiplier;
    if (pbr.useMetallicMap) {
        metallic *= texture(pbr.metallicMap, TexCoords).r;
    }

    // 4. Roughness
    float roughness = pbr.roughnessMultiplier;
    if (pbr.useRoughnessMap) {
        roughness *= texture(pbr.roughnessMap, TexCoords).r;
    }

    // 5. Ambient Occlusion
    float ao = 1.0; // Default to full brightness if no AO map
    if (pbr.useAOMap) {
        ao = texture(pbr.aoMap, TexCoords).r;
    }

    // --- LIGHTING CALCULATIONS ---

    vec3 V = normalize(u_CameraPosition - WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for(int i = 0; i < LightCount; i++) 
    {
        PointLight light = Lights[i];
        vec3 lightPosition = light.Position.xyz;
        vec3 lightColor = light.Color.xyz;
        vec3 L = normalize(lightPosition - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPosition - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColor * attenuation;

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
