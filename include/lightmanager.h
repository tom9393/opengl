#ifndef LIGHTMANAGER
#define LIGHTMANAGER

#include "drawable.h"
#include "ishader.h"
#include "light.h"
#include "mappedlight.h"
#include <vector>

class Lightmanager
{
    public:
    Lightmanager();

    void addLight(ILight *light);
    void addLight(std::vector<ILight*> *light);
    void readShadowMap(ISetLight &shader);
    void writeShadowMap(ISetLight &shader, std::vector<IDrawable*> &drawables);
    void update();

    private:
    std::vector<IMappedLight*> lights;
    unsigned int spotlights = 0;
    unsigned int pointlights = 0;
    unsigned int dirlights = 0;
    unsigned int map = 20;
};

#endif
