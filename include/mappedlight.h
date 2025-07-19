#ifndef MAPPEDLIGHT
#define MAPPEDLIGHT

#include "../include/preprocessor.h"

#include "drawable.h"
#include "ishader.h"
#include "light.h"
#include "shadowmap.h"

class MappedLight : public IDrawable
{
public:
    MappedLight(Light &light, unsigned int lightNumber, unsigned int textureUnit) : light(&light), lightNumber(lightNumber), textureUnit(textureUnit), map(new ShadowMap()), shadowProgram(0) {}
    bool shouldDraw(Shader &shader) override
    {
        return true;
    }
    void draw(Shader &shader) override
    {
        if (shadowProgram == 0) shadowProgram = shader.GetProgram();
        if (shadowProgram == shader.GetProgram()) {
            Info("Writing in shader " << shader.GetProgram() << " light number " << lightNumber);
            map->BindForWriting();
            light->prepareLight(shader);
        } else {
            Info("Reading in shader " << shader.GetProgram() << " light number " << lightNumber << " texture number " << textureUnit);
            map->BindForReading(GL_TEXTURE0 + textureUnit);
            light->setLight(shader, lightNumber, textureUnit);
        }
    }
protected:
    Light* light;
    IShadowMap* map;
    unsigned int lightNumber;
    unsigned int textureUnit;
    unsigned int shadowProgram;
};

#endif
