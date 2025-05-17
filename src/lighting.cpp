
#include "../include/lighting.h"
#include <glm/glm.hpp>

Lighting::Lighting()
{
}

bool Lighting::Init()
{
    if (!Shader::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "/home/tom/Coding/c++/opengl/shaders/default.vert")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "/home/tom/Coding/c++/opengl/shaders/default.frag")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    Model = GetUniformLocation("model");
    View = GetUniformLocation("view");
    Projection = GetUniformLocation("projection");
    materialLoc.AmbientColor = GetUniformLocation("material.ambient");
    materialLoc.DiffuseColor = GetUniformLocation("material.diffuse");
    materialLoc.SpecularColor = GetUniformLocation("material.specular");
    materialLoc.Shininess = GetUniformLocation("material.shininess");

    CameraLocalPosLoc = GetUniformLocation("viewPos");

    dirLightLoc.Ambient = GetUniformLocation("light.ambient");
    dirLightLoc.Diffuse = GetUniformLocation("light.diffuse");
    dirLightLoc.Specular = GetUniformLocation("light.specular");
    dirLightLoc.Constant = GetUniformLocation("light.constant");
    dirLightLoc.Linear = GetUniformLocation("light.linear");
    dirLightLoc.Quadratic = GetUniformLocation("light.quadratic");
    dirLightLoc.Position = GetUniformLocation("light.position");
    dirLightLoc.Direction = GetUniformLocation("light.direction");
    dirLightLoc.Cutoff = GetUniformLocation("light.cutoff");
    dirLightLoc.OuterCutoff = GetUniformLocation("light.outerCutOff");

    if (Model == 0xFFFFFFFF ||
        View == 0xFFFFFFFF ||
        Projection == 0xFFFFFFFF ||
        samplerLoc == 0xFFFFFFFF ||
        samplerSpecularExponentLoc == 0xFFFFFFFF ||
        materialLoc.AmbientColor == 0xFFFFFFFF ||
        materialLoc.DiffuseColor == 0xFFFFFFFF ||
        materialLoc.SpecularColor == 0xFFFFFFFF ||
        CameraLocalPosLoc == 0xFFFFFFFF ||
        dirLightLoc.Ambient == 0xFFFFFFFF ||
        dirLightLoc.Diffuse == 0xFFFFFFFF ||
        dirLightLoc.Specular == 0xFFFFFFFF ||
        dirLightLoc.Constant == 0xFFFFFFFF ||
        dirLightLoc.Linear == 0xFFFFFFFF ||
        dirLightLoc.Quadratic == 0xFFFFFFFF ||
        dirLightLoc.Position == 0xFFFFFFFF ||
        dirLightLoc.Direction == 0xFFFFFFFF ||
        dirLightLoc.Cutoff == 0xFFFFFFFF ||
        dirLightLoc.OuterCutoff == 0xFFFFFFFF)
    {
        return false;
    }

    return true;
}

void Lighting::SetModel(const glm::mat4& model)
{
    glUniformMatrix4fv(Model, 1, GL_FALSE, &model[0][0]);
}

void Lighting::SetView(const glm::mat4& view)
{
    glUniformMatrix4fv(View, 1, GL_FALSE, &view[0][0]);
}

void Lighting::SetProjection(const glm::mat4& projection)
{
    glUniformMatrix4fv(Projection, 1, GL_FALSE, &projection[0][0]);
}


void Lighting::SetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(samplerLoc, TextureUnit);
}

void Lighting::SetTexture(const std::string &name, int value)
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), value); 
}

void Lighting::SetSpecularExponentTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(samplerSpecularExponentLoc, TextureUnit);
}


void Lighting::SetDirectionalLight(const Light& Light)
{
    glUniform3fv(dirLightLoc.Ambient, 1, &Light.ambient[0]); 
    glUniform3fv(dirLightLoc.Diffuse, 1, &Light.diffuse[0]); 
    glUniform3fv(dirLightLoc.Specular, 1, &Light.specular[0]); 
    glUniform3fv(dirLightLoc.Position, 1, &Light.position[0]); 
    glUniform3fv(dirLightLoc.Direction, 1, &Light.direction[0]); 
    glUniform1f(dirLightLoc.Cutoff, Light.cutoff);
    glUniform1f(dirLightLoc.OuterCutoff, Light.outerCutOff);
    glUniform1f(dirLightLoc.Constant, Light.constant);
    glUniform1f(dirLightLoc.Linear, Light.linear);
    glUniform1f(dirLightLoc.Quadratic, Light.quadratic);
}


void Lighting::SetCameraLocalPos(const glm::vec3& CameraLocalPos)
{
    glUniform3f(CameraLocalPosLoc, CameraLocalPos.x, CameraLocalPos.y, CameraLocalPos.z);
}


void Lighting::SetMaterial(const Material& material)
{
    glUniform3fv(materialLoc.AmbientColor, 1, &material.ambient[0]); 
    glUniform3fv(materialLoc.DiffuseColor, 1, &material.diffuse[0]); 
    glUniform3fv(materialLoc.SpecularColor, 1, &material.specular[0]); 
    glUniform1f(materialLoc.Shininess, material.shininess);
}
