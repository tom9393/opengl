#ifndef LIGHT
#define LIGHT

#include "shader.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum LightType
{
    DIRECTIONAL,
    SPOTLIGHT,
    POINTLIGHT
};

class ILight
{
public:
    virtual void setLight(Shader *shader, const std::string &name) = 0;
    virtual LightType getType() = 0;
    virtual ~ILight() = default;

protected:
    LightType type;
};

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

class DirLight : public Light, public ILight
{
public:
    DirLight(glm::vec3 dir = glm::vec3(-2.0f, 4.0f, -1.0f))
    {
        direction = dir;
        float near = 1.0f;
        float far = 25.0f;
	projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near, far);
    }
    glm::vec3 getDirection();
    glm::mat4 getMatrix();
    void setLight(Shader *shader, const std::string &name);
    LightType getType();

private:
    glm::vec3 direction;
    glm::mat4 projection;
    LightType type = DIRECTIONAL;
};

class PointLight : public AttenuatingLight, public ILight
{
public:
    PointLight();
    glm::vec3 getPosition();
    void setPosition(glm::vec3 pos);
    void setLight(Shader *shader, const std::string &name);
    LightType getType();

private:
    glm::vec3 position;
    LightType type = POINTLIGHT;
};

class SpotLight : public AttenuatingLight, public ILight
{
public:
    SpotLight(glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f))
    {
        projection = proj;
        target = glm::vec3(0.0f);
        cutoff = glm::cos(glm::radians(12.5f));
        outerCutoff = glm::cos(glm::radians(17.5f));
    }

    glm::vec3 getDirection();
    void setDirection(glm::vec3 dir);
    glm::vec3 getPosition();
    void setPosition(glm::vec3 pos);
    float getCutoff();
    float getOutercutoff();
    glm::mat4 getMatrix();
    void setTarget(glm::vec3 target);
    glm::vec3 getTarget();
    glm::vec3 getDirectionToTarget(glm::mat4 mat);
    void setLight(Shader *shader, const std::string &name);
    LightType getType();
    
private:
    glm::vec3 direction;
    glm::vec3 position;
    float cutoff;
    float outerCutoff;
    glm::mat4 projection;
    glm::vec3 target;
    LightType type = SPOTLIGHT;
};

#endif
