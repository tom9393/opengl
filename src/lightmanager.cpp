#include "../include/lightmanager.h"

Lightmanager::Lightmanager() {}

void Lightmanager::addLight(ILight *light) {
    IMappedLight* l;

    switch (light->getType()) {
        case DIRECTIONAL: {
            l = new MappedDirectional(*light, dirlights);
            dirlights++;
            break;
        }
        case SPOTLIGHT: {
            l = new MappedSpotlight(*light, spotlights);
            spotlights++;
            break;
        }
        case POINTLIGHT: {
            l = new MappedPointlight(*light, pointlights);
            pointlights++;
            break;
        }
    }
    lights.push_back(l);
}

void Lightmanager::addLight(std::vector<ILight*> *lights)
{
    for (unsigned int i = 0; i < lights->size(); i++) {
        addLight(lights->at(i));
    }
}

void Lightmanager::readShadowMap(ISetLight &shader)
{

    for (unsigned int i = 0; i < lights.size() && map < 32; i++) {
        IMappedLight* light = lights[i];
        light->readShadowMap(shader, map);
        map++;
    }
}

void Lightmanager::writeShadowMap(ISetLight &shader, std::vector<IDrawable*> &drawables)
{
    for (unsigned int i = 0; i < lights.size(); i++) {
        IMappedLight* light = lights[i];
        light->writeShadowMap(shader);
        for (unsigned int j = 0; j < drawables.size(); j++)
        {
            drawables[j]->draw(shader);
        }
    }
}

void Lightmanager::update()
{
}
