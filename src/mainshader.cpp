#include "../include/mainshader.h"
#include <glm/glm.hpp>
#include <string>

Mainshader::Mainshader()
{
}

bool Mainshader::Init()
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

    return true;
}

void Mainshader::setLight(unsigned int index, ILight &light)
{
    switch (light.getType()) {
        case DIRECTIONAL: {
        setDirlight(index, (DirLight&)light);
        break;
    }
        case SPOTLIGHT: {
        setSpotlight(index, (SpotLight&)light);
        break;
    }
        case POINTLIGHT: {
        setPointlight(index, (PointLight&)light);
        break;
    }
    }
}

void Mainshader::setup()
{
    lightmanager->readShadowMap(*this);
}

void Mainshader::draw(std::vector<IDrawable*> &drawables)
{
    for (unsigned int i = 0; i < drawables.size(); i++) {
        drawables[i]->draw(*this);
    }
}

void Mainshader::cleanup()
{
}

void Mainshader::setSpotlight(unsigned int index, SpotLight &light)
{
    std::string prefix = "spotLight[" + std::to_string(index) + "]";

    glUniform3fv(GetUniformLocation((prefix + ".position").c_str()), 1, &light.getPosition()[0]);
    glUniform3fv(GetUniformLocation((prefix + ".direction").c_str()), 1, &light.getDirection()[0]);
    glUniform3fv(GetUniformLocation((prefix + ".ambient").c_str()), 1, &light.getAmbient()[0]);
    glUniform3fv(GetUniformLocation((prefix + ".diffuse").c_str()), 1, &light.getDiffuse()[0]);
    glUniform3fv(GetUniformLocation((prefix + ".specular").c_str()), 1, &light.getSpecular()[0]);

    glUniform1f(glGetUniformLocation(id, (prefix + ".constant").c_str()), light.getConstant());
    glUniform1f(glGetUniformLocation(id, (prefix + ".linear").c_str()), light.getLinear());
    glUniform1f(glGetUniformLocation(id, (prefix + ".quadratic").c_str()), light.getQuadratic());
}

void Mainshader::setPointlight(unsigned int index, PointLight &light)
{
    std::string prefix = "pointLights[" + std::to_string(index) + "].";

    glUniform3fv(GetUniformLocation((prefix + "position").c_str()), 1, &light.getPosition()[0]);
    glUniform3fv(GetUniformLocation((prefix + "ambient").c_str()), 1, &light.getAmbient()[0]);
    glUniform3fv(GetUniformLocation((prefix + "diffuse").c_str()), 1, &light.getDiffuse()[0]);
    glUniform3fv(GetUniformLocation((prefix + "specular").c_str()), 1, &light.getSpecular()[0]);

    glUniform1f(glGetUniformLocation(id, (prefix + "constant").c_str()), light.getConstant());
    glUniform1f(glGetUniformLocation(id, (prefix + "linear").c_str()), light.getLinear());
    glUniform1f(glGetUniformLocation(id, (prefix + "quadratic").c_str()), light.getQuadratic());

    glUniform1f(glGetUniformLocation(id, "material.shininess"), 32.0f);
}

void Mainshader::setDirlight(unsigned int index, DirLight &light)
{
    glUniform3fv(GetUniformLocation("dirLight.direction"), 1, &light.getDirection()[0]); 
    glUniform3fv(GetUniformLocation("dirLight.ambient"), 1, &light.getAmbient()[0]); 
    glUniform3fv(GetUniformLocation("dirLight.diffuse"), 1, &light.getDiffuse()[0]); 
    glUniform3fv(GetUniformLocation("dirLight.specular"), 1, &light.getSpecular()[0]); 
}
