#ifndef MAPPEDLIGHT
#define MAPPEDLIGHT

#include "ishader.h"
#include "light.h"
#include "shadowmap.h"

class IMappedLight {
public:
    IMappedLight(ILight &light, int lightNumber) : light(&light), lightNumber(lightNumber), map(new ShadowMap()) {
        std::cout << map->getDepthMap() << std::endl;
        std::cout << lightNumber << std::endl;
        std::cout << "This is it" << std::endl;
    }
    virtual void setMatrix(IShader &shader) = 0;
    virtual void setLight(IShader &shader, unsigned int map) = 0;
protected:
    ILight* light;
    IShadowMap* map;
    unsigned int lightNumber;
};

class MappedDirectional : public IMappedLight {
public:
    MappedDirectional(ILight &light, int lightNumber) : IMappedLight(light, lightNumber) {};
    void setMatrix(IShader &shader) override {
        map->BindForWriting();
        DirLight *dirlight = (DirLight*) light;
        shader.setMat4("lightSpaceMatrix", dirlight->getMatrix());
    }

    void setLight(IShader &shader, unsigned int textureUnit) override {
        DirLight *dl = (DirLight*) light;
        shader.setLight(0, *dl);
        map->BindForReading(GL_TEXTURE0 + textureUnit);
        shader.setInt("shadowMap", textureUnit);
        dl->setLight((Shader*)&shader, "lightSpaceMatrix");
    }
};

class MappedSpotlight : public IMappedLight {
    public:
    MappedSpotlight(ILight &light, int lightNumber) : IMappedLight(light, lightNumber) {};
    void setMatrix(IShader &shader) override {
        map->BindForWriting();
        SpotLight *spotlight = (SpotLight*) light;
        shader.setMat4("lightSpaceMatrix", spotlight->getMatrix());
    }

    void setLight(IShader &shader, unsigned int textureUnit) override {
        SpotLight *sl = (SpotLight*) light;
        shader.setLight(lightNumber, *sl);
        std::string prefix = "shadowMapSpot[" + std::to_string(lightNumber) + "]";
        std::string prefix2 = "spotLightSpaceMatrices[" + std::to_string(lightNumber) + "]";
        map->BindForReading(GL_TEXTURE0 + textureUnit);
        shader.setInt(prefix, textureUnit);
        sl->setLight((Shader*)&shader, prefix2);
    }
};

class MappedPointlight : public IMappedLight {
    public:
    MappedPointlight(ILight &light, int lightNumber) : IMappedLight(light, lightNumber) {};
    void setMatrix(IShader &shader) override {
        map->BindForWriting();
        PointLight *pointlight = (PointLight*) light;
        shader.setMat4("lightSpaceMatrix", glm::mat4(10.0f));
    }

    void setLight(IShader &shader, unsigned int textureUnit) override {
        PointLight *pl = (PointLight*) light;
        shader.setLight(lightNumber, *pl);
    }
};

#endif
