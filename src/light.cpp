#include "../include/light.h"

Light::Light()
{
    ambient = glm::vec3(0.05f);
    diffuse = glm::vec3(1.0f);
    specular = glm::vec3(1.0f);
}

glm::vec3 Light::getAmbient()
{
    return ambient;
}

glm::vec3 Light::getDiffuse()
{
    return diffuse;
}

glm::vec3 Light::getSpecular()
{
    return specular;
}

AttenuatingLight::AttenuatingLight()
{
    constant = 1.0f;
    linear = 0.09f;
    quadratic = 0.032f;
}

float AttenuatingLight::getConstant()
{
    return constant;
}

float AttenuatingLight::getLinear()
{
    return linear;
}

float AttenuatingLight::getQuadratic()
{
    return quadratic;
}

DirLight::DirLight()
{
    direction = glm::vec3(-2.0f, 4.0f, -1.0f);
}

glm::vec3 DirLight::getDirection()
{
    return direction;
}

PointLight::PointLight()
{
    position = glm::vec3(0.0f);
}

glm::vec3 PointLight::getPosition()
{
    return position;
}

void PointLight::setPosition(glm::vec3 pos)
{
    position = pos;
}

SpotLight::SpotLight()
{
    cutoff = glm::cos(glm::radians(12.5f));
    outerCutoff = glm::cos(glm::radians(17.5f));
}

glm::vec3 SpotLight::getDirection()
{
    return direction;
}

void SpotLight::setDirection(glm::vec3 dir)
{
    direction = dir;
}

float SpotLight::getCutoff()
{
    return cutoff;
}

float SpotLight::getOutercutoff()
{
    return outerCutoff;
}
