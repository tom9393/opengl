#ifndef LIGHTING
#define LIGHTING

#include "shader.h"
#include "material.h"
#include <glm/glm.hpp>

class Light
{
    public:

    glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
    float constant = 0.0f;
    float linear = 0.0f;
    float quadratic = 0.0f;
    float cutoff = 0.0f;
    float outerCutOff = 0.0f;
};

class Lighting : public Shader
{
public:

    Lighting();

    virtual bool Init();

    void SetView(const glm::mat4& View);
    void SetModel(const glm::mat4& Model);
    void SetProjection(const glm::mat4& Projection);
    void SetTextureUnit(unsigned int TextureUnit);
    void SetSpecularExponentTextureUnit(unsigned int TextureUnit);
    void SetDirectionalLight(const Light& Light);
    void SetCameraLocalPos(const glm::vec3& CameraLocalPos);
    void SetMaterial(const Material& material);
    void SetTexture(const std::string &name, int value);

private:

    GLuint Model;
    GLuint View;
    GLuint Projection;
    GLuint samplerLoc;
    GLuint samplerSpecularExponentLoc;
    GLuint CameraLocalPosLoc;

    struct {
        GLuint AmbientColor;
        GLuint DiffuseColor;
        GLuint SpecularColor;
        GLuint Shininess;
    } materialLoc;

    struct {
        GLuint Ambient;
        GLuint Diffuse;
        GLuint Specular;
        GLuint Position;
        GLuint Direction;
        GLuint Constant;
        GLuint Linear;
        GLuint Quadratic;
        GLuint Cutoff;
        GLuint OuterCutoff;
    } dirLightLoc;
};

#endif
