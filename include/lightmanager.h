#ifndef LIGHTMANAGER
#define LIGHTMANAGER

#include "light.h"
#include "mappedlight.h"
#include <vector>

class Lightmanager
{
    public:
    Lightmanager();

    void addLight(ILight *light);
    void readShadowMap(ILightShader &shader);
    void writeShadowMap(ILightShader &shader);

    private:
    std::vector<IMappedLight*> lights;
    unsigned int spotlights = 0;
    unsigned int pointlights = 0;
    unsigned int dirlights = 0;
    unsigned int map = 20;
};

#endif
