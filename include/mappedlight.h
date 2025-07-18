#ifndef MAPPEDLIGHT
#define MAPPEDLIGHT

#include "../include/preprocessor.h"

#include "drawable.h"
#include "ishader.h"
#include "light.h"
#include "shadowmap.h"

class IMappedLight : public IDrawable
{
public:
    IMappedLight(Light &light, unsigned int lightNumber, unsigned int textureUnit) : light(&light), lightNumber(lightNumber), textureUnit(textureUnit), map(new ShadowMap()), shadowProgram(0) {}
    bool shouldDraw(Shader &shader)
    {
        return true;
    }
protected:
    Light* light;
    IShadowMap* map;
    unsigned int lightNumber;
    unsigned int textureUnit;
    unsigned int shadowProgram;
};

class MappedDirectional : public IMappedLight {
public:
    MappedDirectional(Light &light, unsigned int lightNumber, unsigned int textureUnit) : IMappedLight(light, lightNumber, textureUnit) {};

    void draw(Shader &shader) override
    {
        if (shadowProgram == 0) shadowProgram = shader.GetProgram();
        if (shadowProgram == shader.GetProgram()) {
            Info("Writing in dirlight shader " << shader.GetProgram() << " light number " << lightNumber);
            map->BindForWriting();
            light->prepareLight(shader);
        } else {
            Info("Reading in dirlight shader " << shader.GetProgram() << " light number " << lightNumber << " texture number " << textureUnit);
            map->BindForReading(GL_TEXTURE0 + textureUnit);
            light->setLight(shader, lightNumber, textureUnit);
        }
    }
};

class MappedSpotlight : public IMappedLight {
public:
    MappedSpotlight(Light &light, unsigned int lightNumber, unsigned int textureUnit) : IMappedLight(light, lightNumber, textureUnit) {};

    void draw(Shader &shader) override
    {
        if (shadowProgram == 0) shadowProgram = shader.GetProgram();
        if (shadowProgram == shader.GetProgram()) {
            Info("Writing in spotlight shader " << shader.GetProgram() << " light number " << lightNumber);
            map->BindForWriting();
            light->prepareLight(shader);
        } else {
            Info("Reading in spotlight shader " << shader.GetProgram() << " light number " << lightNumber << " texture number " << textureUnit);
            map->BindForReading(GL_TEXTURE0 + textureUnit);
            light->setLight(shader, lightNumber, textureUnit);
        }
    }
};

class MappedPointlight : public IMappedLight {
public:
    MappedPointlight(Light &light, unsigned int lightNumber, unsigned int textureUnit) : IMappedLight(light, lightNumber, textureUnit) {};

    void draw(Shader &shader) override
    {
        if (shadowProgram == 0) shadowProgram = shader.GetProgram();
        if (shadowProgram == shader.GetProgram()) {
            Info("Writing in pointlight shader " << shader.GetProgram() << " light number " << lightNumber);
            map->BindForWriting();
            light->prepareLight(shader);
        } else {
            Info("Reading in pointlight shader " << shader.GetProgram() << " light number " << lightNumber << " texture number " << textureUnit);
            // Point lights are not implemented yet, ignore them
            map->BindForReading(GL_TEXTURE0 + textureUnit);
            light->setLight(shader, lightNumber, textureUnit);
        }
    }
};

#endif
