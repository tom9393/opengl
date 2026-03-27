#version 460 core

layout (location = 0) out vec4 FragColor;

struct PBR {
    sampler2D albedoMap;
    sampler2D normalMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D aoMap;
};

struct VertexOutput
{
    vec3 FragPos;
    vec2 TexCoord;
    mat3 TBN;
};

layout (location = 0) in VertexOutput Input;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
    vec3 u_CameraPosition;
};

uniform vec3 lightPositions[4]; 
uniform vec3 lightColors[4];

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

    // Extract map data
    vec3 albedo     = pow(texture(pbr.albedoMap, TexCoords).rgb, vec3(2.2)); // Convert to linear space
    float metallic  = texture(pbr.metallicMap, TexCoords).r;
    float roughness = texture(pbr.roughnessMap, TexCoords).r;
    float ao        = texture(pbr.aoMap, TexCoords).r;

    vec3 WorldPos = Input.FragPos;
    // Extract and transform normal
    vec3 normal = texture(pbr.normalMap, TexCoords).rgb;
    normal = normal * 2.0 - 1.0; // Map from [0,1] to [-1,1]
    vec3 N = normalize(TBN * normal);

    vec3 V = normalize(u_CameraPosition - WorldPos);

    // Base reflectivity (F0). 0.04 is a good default for non-metals (dielectrics).
    // If it's a metal, use the albedo color as the base reflectivity.
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    // Loop through all lights
    for(int i = 0; i < 4; ++i) 
    {
        // Calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is energy of light that gets reflected. kD is light that gets refracted.
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  // Metals absorb all refracted light, so they have no diffuse lighting

        // Add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);        
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }   

    // Ambient lighting (Replace this with IBL later!)
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR Tonemapping and Gamma Correction
    color = color / (color + vec3(1.0)); // Reinhard tone mapping
    color = pow(color, vec3(1.0/2.2));   // Gamma correction 

    FragColor = vec4(color, 1.0);
}
