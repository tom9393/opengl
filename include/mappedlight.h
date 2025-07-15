#ifndef MAPPEDLIGHT
#define MAPPEDLIGHT

#include "drawable.h"
#include "ishader.h"
#include "light.h"
#include "shadowmap.h"

class IMappedLight : public IDrawable
{
public:
    IMappedLight(ILight &light, unsigned int lightNumber, unsigned int textureUnit) : light(&light), lightNumber(lightNumber), textureUnit(textureUnit), map(new ShadowMap()), shadowProgram(0) {}
protected:
    ILight* light;
    IShadowMap* map;
    unsigned int lightNumber;
    unsigned int textureUnit;
    unsigned int shadowProgram;
};

class MappedDirectional : public IMappedLight {
public:
    MappedDirectional(ILight &light, unsigned int lightNumber, unsigned int textureUnit) : IMappedLight(light, lightNumber, textureUnit) {};

    void draw(Shader &shader) override
    {
        if (shadowProgram == 0) shadowProgram = shader.GetProgram();
        if (shadowProgram == shader.GetProgram()) {
            map->BindForWriting();
            DirLight *dirlight = (DirLight*) light;
            shader.setMat4("lightSpaceMatrix", dirlight->getMatrix());
        } else {
            DirLight *dl = (DirLight*) light;
            // Index unused for now
            unsigned int idx = 0;

            glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.direction"), 1, &dl->getDirection()[0]); 
            glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.ambient"), 1, &dl->getAmbient()[0]); 
            glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.diffuse"), 1, &dl->getDiffuse()[0]); 
            glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.specular"), 1, &dl->getSpecular()[0]); 
            map->BindForReading(GL_TEXTURE0 + textureUnit);
            shader.setInt("shadowMap", textureUnit);
            dl->setLight(&shader, "lightSpaceMatrix");
        }
    }
};

class MappedSpotlight : public IMappedLight {
public:
    MappedSpotlight(ILight &light, unsigned int lightNumber, unsigned int textureUnit) : IMappedLight(light, lightNumber, textureUnit) {};

    void draw(Shader &shader) override
    {
        if (shadowProgram == 0) shadowProgram = shader.GetProgram();
        if (shadowProgram == shader.GetProgram()) {
            map->BindForWriting();
            SpotLight *spotlight = (SpotLight*) light;
            shader.setMat4("lightSpaceMatrix", spotlight->getMatrix());
        } else {
            SpotLight *sl = (SpotLight*) light;
            std::string prefix = "spotLight[" + std::to_string(lightNumber) + "]";
            std::cout << prefix << std::endl;

            glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".position").c_str()), 1, &sl->getPosition()[0]);
            glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".ambient").c_str()), 1, &sl->getAmbient()[0]);
            glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".diffuse").c_str()), 1, &sl->getDiffuse()[0]);
            glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".specular").c_str()), 1, &sl->getSpecular()[0]);
            glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".constant").c_str()), sl->getConstant());
            glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".linear").c_str()), sl->getLinear());
            glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".quadratic").c_str()), sl->getQuadratic());
            std::string prefix2 = "shadowMapSpot[" + std::to_string(lightNumber) + "]";
            std::string prefix3 = "spotLightSpaceMatrices[" + std::to_string(lightNumber) + "]";
            map->BindForReading(GL_TEXTURE0 + textureUnit);
            shader.setInt(prefix2, textureUnit);
            sl->setLight(&shader, prefix3);
        }
    }
};

class MappedPointlight : public IMappedLight {
public:
    MappedPointlight(ILight &light, unsigned int lightNumber, unsigned int textureUnit) : IMappedLight(light, lightNumber, textureUnit) {};

    void draw(Shader &shader) override
    {
        if (shadowProgram == 0) shadowProgram = shader.GetProgram();
        if (shadowProgram == shader.GetProgram()) {
            map->BindForWriting();
            PointLight *pointlight = (PointLight*) light;
            shader.setMat4("lightSpaceMatrix", glm::mat4(10.0f));
        } else {
            PointLight *pl = (PointLight*) light;
            std::string prefix = "pointLights[" + std::to_string(lightNumber) + "].";

            glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + "position").c_str()), 1, &pl->getPosition()[0]);
            glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + "ambient").c_str()), 1, &pl->getAmbient()[0]);
            glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + "diffuse").c_str()), 1, &pl->getDiffuse()[0]);
            glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + "specular").c_str()), 1, &pl->getSpecular()[0]);

            glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + "constant").c_str()), pl->getConstant());
            glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + "linear").c_str()), pl->getLinear());
            glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + "quadratic").c_str()), pl->getQuadratic());

            glUniform1f(glGetUniformLocation(shader.GetProgram(), "material.shininess"), 32.0f);
        }
    }
};

#endif
