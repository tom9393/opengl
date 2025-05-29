#include "../include/light.h"
#include <glm/ext/matrix_transform.hpp>

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

glm::vec3 DirLight::getDirection()
{
    return direction;
}

glm::mat4 DirLight::getMatrix()
{
    glm::mat4 view = glm::lookAt(getDirection(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f,  0.0f));
    return projection * view;


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

void SpotLight::setTarget(glm::vec3 target)
{
    target = target;
}

glm::vec3 SpotLight::getTarget()
{
    return target;
}

glm::mat4 SpotLight::getMatrix()
{
    glm::mat4 view = glm::lookAt(getPosition(), getTarget(), glm::vec3(0.0f, 1.0f,  0.0f));
    return projection * view;
}

glm::vec3 SpotLight::getDirectionToTarget(glm::mat4 mat)
{
    glm::vec3 positionInversed = glm::vec3(glm::inverse(mat)[3]);
    return glm::normalize(target - positionInversed);

}
