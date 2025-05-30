#include "../include/lighting.h"
#include <glm/glm.hpp>
#include <string>

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

    return true;
}

void Lighting::setSpotlight(unsigned int index, SpotLight &light)
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

void Lighting::setPointlight(unsigned int index, PointLight &light)
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

void Lighting::setDirlight(unsigned int index, DirLight &light)
{
        glUniform3fv(GetUniformLocation("dirLight.direction"), 1, &light.getDirection()[0]); 
        glUniform3fv(GetUniformLocation("dirLight.ambient"), 1, &light.getAmbient()[0]); 
        glUniform3fv(GetUniformLocation("dirLight.diffuse"), 1, &light.getDiffuse()[0]); 
        glUniform3fv(GetUniformLocation("dirLight.specular"), 1, &light.getSpecular()[0]); 
}

void Lighting::setup()
{
    for (unsigned int i = 0; i < dirLights.size(); i++)
    {
        DirLight *light = dirLights[i];
        ShadowMap &shadowMap = dirMaps[0];
        setDirlight(i, *light);
	shadowMap.BindForReading(GL_TEXTURE25);
        setInt("shadowMap", 25);
        setMat4("lightSpaceMatrix", light->getMatrix());
    }
    unsigned int text = 26;
    for (unsigned int i = 0; i < spotLights.size(); i++)
    {
        SpotLight *light = spotLights[i];
        ShadowMap &shadowMap = shadowMaps[i];
        setSpotlight(i, *light);

        std::string prefix = "shadowMapSpot[" + std::to_string(i) + "]";
        std::string prefix2 = "spotLightSpaceMatrices[" + std::to_string(i) + "]";
        shadowMap.BindForReading(GL_TEXTURE0 + text);
        setInt(prefix, text);
        setMat4(prefix2, light->getMatrix());

        text++;
    }
    for (unsigned int i = 0; i < pointLights.size(); i++)
    {
        PointLight *light = pointLights[i];
        setPointlight(i, *light);
    }
}

void Lighting::draw()
{
    setup();
    for (unsigned int i = 0; i < objs.size(); i++)
    {
        Obj &object = objs[i];
        object.draw(*this);
    }
}

void Lighting::cleanup()
{
}

void Lighting::addDirLight(DirLight *light)
{
    dirLights.push_back(light);
}
void Lighting::addPointLight(PointLight *light)
{
    pointLights.push_back(light);
}
void Lighting::addSpotLight(SpotLight *light)
{
    spotLights.push_back(light);
}
void Lighting::addShadowMap(ShadowMap &shadowMap)
{
    shadowMaps.push_back(shadowMap);
}
void Lighting::addDirMap(ShadowMap &shadowMap)
{
    dirMaps.push_back(shadowMap);
}
void Lighting::addObj(Obj &object)
{
    objs.push_back(object);
}
