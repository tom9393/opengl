#ifndef LIGHTING
#define LIGHTING

#include "drawable.h"
#include "ishader.h"
#include "light.h"
#include <glm/glm.hpp>

class Mainshader : public ISetViewPosition
{
public:

    Mainshader();

    virtual bool Init();

    void setup();
    void draw(std::vector<IDrawable*> &drawables);
    void cleanup();
};

#endif
