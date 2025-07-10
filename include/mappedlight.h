#ifndef MAPPEDLIGHT
#define MAPPEDLIGHT

#include "light.h"
#include "lighting.h"
#include "shadowmap.h"

class IMappedLight {

public:
    IMappedLight(ILight &light, int lightNumber) : light(&light), lightNumber(lightNumber) {
        map = new ShadowMap();
    }
    ILight* getLight() {
        return light;
    }
    void bindForReading(GLenum textureUnit) {
        map->BindForReading(textureUnit);
    }
    void bindForWriting() {
        map->BindForWriting();
    }
    virtual void setMatrix(Lighting &shader);
    virtual void setLight(Lighting &shader, unsigned int map);
protected:
    ILight* light;
    IShadowMap* map;
    unsigned int lightNumber;
};

class MappedDirectional : public IMappedLight {
public:
    void setMatrix(Lighting &shader) override {
        DirLight *dirlight = (DirLight*) light;
        shader.setMat4("lightSpaceMatrix", dirlight->getMatrix());
    }

    void setLight(Lighting &shader, unsigned int textureUnit) override {
        DirLight *dl = (DirLight*) light;
        shader.setDirlight(0, *dl);
        map->BindForReading(textureUnit);
        shader.setInt("shadowMap", textureUnit);
        dl->setLight((Shader*)this, "lightSpaceMatrix");
    }
};

class MappedSpotlight : public IMappedLight {
    void setMatrix(Lighting &shader) override {
        SpotLight *spotlight = (SpotLight*) light;
        shader.setMat4("lightSpaceMatrix", spotlight->getMatrix());
    }

    void setLight(Lighting &shader, unsigned int textureUnit) override {
        SpotLight *sl = (SpotLight*) light;
        shader.setSpotlight(lightNumber, *sl);
        std::string prefix = "shadowMapSpot[" + std::to_string(lightNumber) + "]";
        std::string prefix2 = "spotLightSpaceMatrices[" + std::to_string(lightNumber) + "]";
        map->BindForReading(textureUnit);
        shader.setInt(prefix, textureUnit);
        sl->setLight((Shader*)this, prefix2);
    }
};

class MappedPointlight : public IMappedLight {
    void setMatrix(Lighting &shader) override {
        PointLight *pointlight = (PointLight*) light;
        shader.setMat4("lightSpaceMatrix", glm::mat4(0.0f));
    }

    void setLight(Lighting &shader, unsigned int textureUnit) override {
        PointLight *pl = (PointLight*) light;
        shader.setPointlight(lightNumber, *pl);
    }
};

#endif
