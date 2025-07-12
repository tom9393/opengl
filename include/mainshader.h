#ifndef LIGHTING
#define LIGHTING

#include "drawable.h"
#include "ishader.h"
#include "lightmanager.h"
#include "light.h"
#include <glm/glm.hpp>

class Mainshader : public ILightShader
{
public:

    Mainshader();

    virtual bool Init();

    void setup();
    void draw(std::vector<IDrawable*> &drawables);
    void cleanup();

    Lightmanager* lightmanager;

protected:
    void setLight(unsigned int index, ILight &light);
private:
    void setSpotlight(unsigned int index, SpotLight &light);
    void setPointlight(unsigned int index, PointLight &light);
    void setDirlight(unsigned int index, DirLight &light);
};

#endif
