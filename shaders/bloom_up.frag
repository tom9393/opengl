#version 460 core
// 9-tap tent filter upsample, additively blended into the larger mip.
layout (location = 0) out vec3 FragColor;

in vec2 TexCoords;

layout(binding = 0) uniform sampler2D u_SrcTexture;
uniform float u_FilterRadius;

void main()
{
    float x = u_FilterRadius;
    float y = u_FilterRadius;

    vec3 a = texture(u_SrcTexture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 b = texture(u_SrcTexture, vec2(TexCoords.x,     TexCoords.y + y)).rgb;
    vec3 c = texture(u_SrcTexture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;

    vec3 d = texture(u_SrcTexture, vec2(TexCoords.x - x, TexCoords.y)).rgb;
    vec3 e = texture(u_SrcTexture, vec2(TexCoords.x,     TexCoords.y)).rgb;
    vec3 f = texture(u_SrcTexture, vec2(TexCoords.x + x, TexCoords.y)).rgb;

    vec3 g = texture(u_SrcTexture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 h = texture(u_SrcTexture, vec2(TexCoords.x,     TexCoords.y - y)).rgb;
    vec3 i = texture(u_SrcTexture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    vec3 upsample = e * 4.0;
    upsample += (b + d + f + h) * 2.0;
    upsample += (a + c + g + i);
    upsample *= 1.0 / 16.0;

    FragColor = upsample;
}
