#ifndef LIGHT
#define LIGHT

#include <glm/glm.hpp>

class Light
{
public:
    Light();

    glm::vec3 getAmbient();
    glm::vec3 getDiffuse();
    glm::vec3 getSpecular();

private:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

};

class AttenuatingLight : public Light
{
public:
    AttenuatingLight();

    float getConstant();
    float getLinear();
    float getQuadratic();

private:
    float constant;
    float linear;
    float quadratic;

};

class DirLight : public Light
{
public:
    DirLight();
    glm::vec3 getDirection();

private:
    glm::vec3 direction;
};

class PointLight : public AttenuatingLight
{
public:
    PointLight();
    glm::vec3 getPosition();
    void setPosition(glm::vec3 pos);

private:
    glm::vec3 position;
};

class SpotLight : public PointLight
{
public:
    SpotLight();
    glm::vec3 getDirection();
    void setDirection(glm::vec3 dir);
    float getCutoff();
    float getOutercutoff();
    
private:
    glm::vec3 direction;
    float cutoff;
    float outerCutoff;
};

#endif
