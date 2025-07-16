#ifndef LIGHTMANAGER
#define LIGHTMANAGER

#include "drawable.h"
#include "ishader.h"
#include "light.h"
#include "mappedlight.h"
#include <vector>
#include "tree.h"

class Lightmanager
{
    public:
    Lightmanager();

    void addLight(ILight *light);
    void addLight(std::vector<ILight*> *light);
    void addDrawable(IDrawable *drawable);
    void addDrawable(std::vector<IDrawable*> *drawables);
    std::vector<Batch*> populate(Shader &shadow, Shader &light);

private:
    std::vector<IDrawable*> lights;
    std::vector<IDrawable*> drawables;
    unsigned int spotlights = 0;
    unsigned int pointlights = 0;
    unsigned int dirlights = 0;
    unsigned int map = 20;
};

#endif
