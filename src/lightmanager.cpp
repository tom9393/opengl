#include "../include/lightmanager.h"

Lightmanager::Lightmanager() {}

void Lightmanager::addLight(ILight *light) {
    IMappedLight* l;

    switch (light->getType()) {
        case DIRECTIONAL: {
            l = new MappedDirectional(*light, dirlights, map);
            dirlights++;
            break;
        }
        case SPOTLIGHT: {
            l = new MappedSpotlight(*light, spotlights, map);
            spotlights++;
            break;
        }
        case POINTLIGHT: {
            l = new MappedPointlight(*light, pointlights, map);
            pointlights++;
            break;
        }
    }
    lights.push_back(l);
    map++; // Be careful not to exceed 32
}

void Lightmanager::addLight(std::vector<ILight*> *lights)
{
    for (unsigned int i = 0; i < lights->size(); i++) {
        addLight(lights->at(i));
    }
}

void Lightmanager::addDrawable(IDrawable* drawable)
{
    drawables.push_back(drawable);
}

void Lightmanager::addDrawable(std::vector<IDrawable*> *drawables)
{
    for (unsigned int i = 0; i < drawables->size(); i++) {
        addDrawable(drawables->at(i));
    }
}

void Lightmanager::populate()
{
    for (unsigned int i = 0; i < drawables.size(); i++)
    {
        for (unsigned int j = 0; j < lights.size(); j++)
        {
            lights[j]->addChild(drawables[i]);
        }
    }
}

std::vector<IDrawable*> Lightmanager::getLights()
{
    return lights;
}
