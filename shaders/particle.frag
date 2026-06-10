#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 v_TexCoord;
in vec4 v_Color;

uniform sampler2D u_SceneDepth;
uniform sampler2D u_Texture;
uniform bool u_UseTexture;
uniform bool u_Additive;
uniform vec2 u_ViewportSize;

void main()
{
    // Manual depth test against the G-buffer: this pass's target has no depth
    // attachment, but both depths share the same projection and viewport.
    float sceneDepth = texture(u_SceneDepth, gl_FragCoord.xy / u_ViewportSize).r;
    if (gl_FragCoord.z > sceneDepth) discard;

    vec4 color = v_Color;
    if (u_UseTexture)
    {
        color *= texture(u_Texture, v_TexCoord);
    }
    else
    {
        // Procedural soft circle.
        float dist = length(v_TexCoord - vec2(0.5));
        color.a *= smoothstep(0.5, 0.15, dist);
    }

    if (color.a <= 0.001) discard;

    // Premultiply for additive (GL_ONE, GL_ONE) so alpha still fades the contribution.
    if (u_Additive) color.rgb *= color.a;

    FragColor = color;
}
