#version 460 core
layout (location = 0) out vec4 FragColor;

in vec3 LocalPos;

uniform samplerCube u_EnvironmentMap;

const float PI = 3.14159265359;

void main()
{
    // The world vector acts as the normal of a tangent surface from the
    // origin, aligned to LocalPos. Given this normal, calculate all
    // incoming radiance of the environment. The result is the radiance of
    // light coming from -Normal direction, which is the irradiance used in
    // the diffuse part of the IBL lighting.
    vec3 N = normalize(LocalPos);

    vec3 irradiance = vec3(0.0);

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += texture(u_EnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    FragColor = vec4(irradiance, 1.0);
}
