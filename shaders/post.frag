#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout(binding = 0) uniform sampler2D u_SceneColor;  // HDR scene
layout(binding = 1) uniform sampler2D u_BloomColor;  // bloom result (mip 0 of the chain)

uniform float u_Exposure;
uniform float u_BloomIntensity;
uniform int   u_BloomEnabled;
uniform float u_VignetteStrength;
uniform float u_ChromaticAberration;

// ACES filmic tonemap (Narkowicz approximation).
vec3 ACESFilm(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec2 uv = TexCoords;

    // Chromatic aberration: offset each channel radially from the centre.
    vec3 hdr;
    if (u_ChromaticAberration > 0.0)
    {
        vec2 dir = uv - vec2(0.5);
        vec2 offset = dir * u_ChromaticAberration * 0.01;
        hdr.r = texture(u_SceneColor, uv + offset).r;
        hdr.g = texture(u_SceneColor, uv).g;
        hdr.b = texture(u_SceneColor, uv - offset).b;
    }
    else
    {
        hdr = texture(u_SceneColor, uv).rgb;
    }

    if (u_BloomEnabled == 1)
    {
        vec3 bloom = texture(u_BloomColor, uv).rgb;
        hdr = mix(hdr, bloom, clamp(u_BloomIntensity, 0.0, 1.0));
    }

    // Exposure + ACES tonemap (HDR -> LDR).
    vec3 color = ACESFilm(hdr * u_Exposure);

    // Vignette.
    if (u_VignetteStrength > 0.0)
    {
        vec2 d = uv - vec2(0.5);
        float vignette = smoothstep(0.8, 0.2, length(d) * (1.0 + u_VignetteStrength));
        color *= mix(1.0, vignette, clamp(u_VignetteStrength, 0.0, 1.0));
    }

    // Gamma correction.
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
