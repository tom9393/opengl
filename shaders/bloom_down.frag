#version 460 core
// 13-tap downsample filter (Call of Duty / Sledgehammer "Next Generation Post
// Processing"). The first mip applies a soft threshold + Karis average to tame
// fireflies before the bloom is blurred.
layout (location = 0) out vec3 FragColor;

in vec2 TexCoords;

uniform sampler2D u_SrcTexture;
uniform vec2 u_SrcResolution;
uniform float u_Threshold;
uniform int u_FirstMip; // 1 on the first downsample, 0 otherwise

// Karis weight: luma-weighted average to reduce flickering on the first pass.
float Luma(vec3 c) { return dot(c, vec3(0.2126, 0.7152, 0.0722)); }
float KarisWeight(vec3 c) { return 1.0 / (1.0 + Luma(c)); }

void main()
{
    vec2 srcTexelSize = 1.0 / u_SrcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    vec3 a = texture(u_SrcTexture, vec2(TexCoords.x - 2*x, TexCoords.y + 2*y)).rgb;
    vec3 b = texture(u_SrcTexture, vec2(TexCoords.x,       TexCoords.y + 2*y)).rgb;
    vec3 c = texture(u_SrcTexture, vec2(TexCoords.x + 2*x, TexCoords.y + 2*y)).rgb;

    vec3 d = texture(u_SrcTexture, vec2(TexCoords.x - 2*x, TexCoords.y)).rgb;
    vec3 e = texture(u_SrcTexture, vec2(TexCoords.x,       TexCoords.y)).rgb;
    vec3 f = texture(u_SrcTexture, vec2(TexCoords.x + 2*x, TexCoords.y)).rgb;

    vec3 g = texture(u_SrcTexture, vec2(TexCoords.x - 2*x, TexCoords.y - 2*y)).rgb;
    vec3 h = texture(u_SrcTexture, vec2(TexCoords.x,       TexCoords.y - 2*y)).rgb;
    vec3 i = texture(u_SrcTexture, vec2(TexCoords.x + 2*x, TexCoords.y - 2*y)).rgb;

    vec3 j = texture(u_SrcTexture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 k = texture(u_SrcTexture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;
    vec3 l = texture(u_SrcTexture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 m = texture(u_SrcTexture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    vec3 downsample;
    if (u_FirstMip == 1)
    {
        // Karis-weighted groups to suppress fireflies.
        vec3 group0 = (a + b + d + e) * 0.25;
        vec3 group1 = (b + c + e + f) * 0.25;
        vec3 group2 = (d + e + g + h) * 0.25;
        vec3 group3 = (e + f + h + i) * 0.25;
        vec3 group4 = (j + k + l + m) * 0.25;

        float w0 = KarisWeight(group0);
        float w1 = KarisWeight(group1);
        float w2 = KarisWeight(group2);
        float w3 = KarisWeight(group3);
        float w4 = KarisWeight(group4);
        float wSum = (w0 + w1 + w2 + w3) * 0.125 + w4 * 0.5;

        downsample = (group0 * w0 + group1 * w1 + group2 * w2 + group3 * w3) * 0.125
                   + group4 * w4 * 0.5;
        downsample /= max(wSum, 0.0001);

        // Soft threshold: keep only the bright parts of the image.
        float brightness = max(downsample.r, max(downsample.g, downsample.b));
        float contribution = max(brightness - u_Threshold, 0.0) / max(brightness, 0.0001);
        downsample *= contribution;
    }
    else
    {
        downsample = e * 0.125;
        downsample += (a + c + g + i) * 0.03125;
        downsample += (b + d + f + h) * 0.0625;
        downsample += (j + k + l + m) * 0.125;
    }

    FragColor = max(downsample, vec3(0.0));
}
