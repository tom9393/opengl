#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gPBR;

uniform sampler2DArray u_DirSpotShadowMaps;
uniform samplerCubeArray u_PointShadowCubemaps;
uniform float u_FarPlane;

// Image-based lighting
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D   u_BrdfLUT;
uniform samplerCube u_Skybox;
uniform bool        u_HasEnvironment;
uniform float       u_EnvIntensity;
uniform float       u_PrefilterMaxLod;
uniform mat4        u_InverseViewProjection;

struct Light {
    vec4 PositionType;
    vec4 ColorInner;
    vec4 DirectionOuter;
    mat4 LightSpaceMatrix;
    ivec4 Params;
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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// World-space view ray through the current pixel (for skybox background).
vec3 ViewRayDirection(vec2 uv) {
    vec4 clip = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
    vec4 world = u_InverseViewProjection * clip;
    world /= world.w;
    return normalize(world.xyz - u_CameraPosition);
}

// --- Shadow Calculation Functions ---
// float ShadowCalculation2D(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadowIndex)
// {
//     if (shadowIndex < 0) return 0.0; // Light doesn't cast shadows

//     vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//     projCoords = projCoords * 0.5 + 0.5;
    
//     if(projCoords.z > 1.0) return 0.0;
    
//     float currentDepth = projCoords.z;
//     float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    
//     float shadow = 0.0;
//     // textureSize for 2DArray returns vec3(width, height, layers)
//     vec2 texelSize = 1.0 / textureSize(u_DirSpotShadowMaps, 0).xy; 
//     for(int x = -1; x <= 1; ++x) {
//         for(int y = -1; y <= 1; ++y) {
//             // Note the vec3 texture coord: (u, v, layer)
//             float pcfDepth = texture(u_DirSpotShadowMaps, vec3(projCoords.xy + vec2(x, y) * texelSize, shadowIndex)).r;
//             shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
//         }    
//     }
//     return shadow / 9.0;
// }

float ShadowCalculation2D(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadowIndex)
{
    if (shadowIndex < 0) return 0.0;

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0) return 0.0;
    
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    float shadow = 0.0;
    
    vec2 texelSize = 1.0 / textureSize(u_DirSpotShadowMaps, 0).xy;

    // Poisson disk offsets for smoother blurring
    vec2 poissonDisk[4] = vec2[](
        vec2( -0.94201624, -0.39906216 ),
        vec2( 0.94558609, -0.76890725 ),
        vec2( -0.094184101, -0.92938870 ),
        vec2( 0.34495938, 0.29387760 )
    );
    
    // PCF with variable blur radius
    float pcfFilterSize = 2.0; // Increase this to make the shadow softer
    
    for (int i = 0; i < 4; i++) {
        float pcfDepth = texture(u_DirSpotShadowMaps, vec3(projCoords.xy + poissonDisk[i] * texelSize * pcfFilterSize, shadowIndex)).r; 
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
    }
    
    return shadow / 4.0;
}

// float ShadowCalculationPoint(vec3 fragPos, vec3 lightPos, int shadowIndex) {
//     if (shadowIndex < 0) return 0.0;

//     vec3 fragToLight = fragPos - lightPos;
//     float currentDepth = length(fragToLight);
    
//     // Note the vec4 texture coord for CubeArray: (x, y, z, layer)
//     float closestDepth = texture(u_PointShadowCubemaps, vec4(fragToLight, shadowIndex)).r;
//     closestDepth *= u_FarPlane;
    
//     float bias = 0.05; 
//     return currentDepth - bias > closestDepth ? 1.0 : 0.0;
// }

const vec3 gridSamplingDisk[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float ShadowCalculationPoint(vec3 fragPos, vec3 lightPos, int shadowIndex) {
    if (shadowIndex < 0) return 0.0;
    
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    
    float shadow = 0.0;
    float bias = 0.2; // Slightly higher bias to prevent acne with PCF
    int samples = 20;
    
    // Scale the blur radius based on how far the viewer is to make it look natural
    float viewDistance = length(u_CameraPosition - fragPos);
    float diskRadius = (1.0 + (viewDistance / u_FarPlane)) / 25.0;
    
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(u_PointShadowCubemaps, vec4(fragToLight + gridSamplingDisk[i] * diskRadius, shadowIndex)).r;
        closestDepth *= u_FarPlane;   // Undo mapping [0;1]
        
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    
    return shadow / float(samples);
}

void main()
{
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;
    vec3 N = texture(gNormal, TexCoords).rgb;

    // Background pixels (no geometry) draw the skybox. The G-buffer clears
    // normals to zero, so a near-zero normal flags an empty pixel.
    if (dot(N, N) < 0.5) {
        vec3 bg = u_HasEnvironment
            ? texture(u_Skybox, ViewRayDirection(TexCoords)).rgb
            : vec3(0.2);
        FragColor = vec4(bg, 1.0);
        return;
    }

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
        int shadowIndex = light.Params.x;

        if (type == 1) { // Directional
            L = normalize(-light.DirectionOuter.xyz);
            vec4 fragPosLightSpace = light.LightSpaceMatrix * vec4(WorldPos, 1.0);
            shadow = ShadowCalculation2D(fragPosLightSpace, N, L, shadowIndex);
        }
        else if (type == 0) { // Point
            L = normalize(light.PositionType.xyz - WorldPos);
            float distance = length(light.PositionType.xyz - WorldPos);
            attenuation = 1.0 / (distance * distance);
            shadow = ShadowCalculationPoint(WorldPos, light.PositionType.xyz, shadowIndex);
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
            shadow = ShadowCalculation2D(fragPosLightSpace, N, L, shadowIndex);
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

    vec3 ambient;
    if (u_HasEnvironment) {
        // Image-based lighting: diffuse irradiance + specular prefilter * BRDF.
        vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

        vec3 irradiance = texture(u_IrradianceMap, N).rgb;
        vec3 diffuse = irradiance * albedo;

        vec3 R = reflect(-V, N);
        vec3 prefiltered = textureLod(u_PrefilterMap, R, roughness * u_PrefilterMaxLod).rgb;
        vec2 brdf = texture(u_BrdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specular = prefiltered * (F * brdf.x + brdf.y);

        ambient = (kD * diffuse + specular) * ao * u_EnvIntensity;
    } else {
        ambient = vec3(0.03) * albedo * ao;
    }

    // Output linear HDR; tonemapping and gamma happen in the post-process pass.
    FragColor = vec4(ambient + Lo, 1.0);
}