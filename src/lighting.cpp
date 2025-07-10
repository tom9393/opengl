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
    unsigned int spot = 0;
    unsigned int point = 0;
    unsigned int map = 20;

    for (unsigned int i = 0; i < shadowMaps.size() && map < 32; i++)
    {
        ILight *light = lights[i];
        IShadowMap *shadowMap = shadowMaps[i];

        switch(light->getType())
        {
            case DIRECTIONAL: { // For now there will only ever be one directional light
                DirLight *dl = (DirLight*) light;
                setDirlight(0, *dl);
                shadowMap->BindForReading(GL_TEXTURE0 + map);
                setInt("shadowMap", map);
                dl->setLight((Shader*)this, "lightSpaceMatrix");
                break;
            }
            case SPOTLIGHT: {
                SpotLight *sl = (SpotLight*) light;
                setSpotlight(spot, *sl);
                std::string prefix = "shadowMapSpot[" + std::to_string(spot) + "]";
                std::string prefix2 = "spotLightSpaceMatrices[" + std::to_string(spot) + "]";
                shadowMap->BindForReading(GL_TEXTURE0 + map);
                setInt(prefix, map);
                sl->setLight((Shader*)this, prefix2);
                spot++;
                break;
            }
            case POINTLIGHT: {
                PointLight *pl = (PointLight*) light;
                setPointlight(point, *pl);
                point++;
                break;
            }
        }
        map++;
    }
}

void Lighting::draw()
{
    for (unsigned int i = 0; i < objs.size(); i++)
    {
        Obj *object = objs[i];
        object->draw(*this);
    }
}

void Lighting::cleanup()
{
}
void Lighting::addShadowMap(IShadowMap *shadowMap)
{
    shadowMaps.push_back(shadowMap);
}
void Lighting::addObj(Obj *object)
{
    objs.push_back(object);
}
void Lighting::addLight(ILight *light)
{
    lights.push_back(light);
}
