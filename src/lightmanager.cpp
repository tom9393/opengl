#include "../include/lightmanager.h"

Lightmanager::Lightmanager() {}

void Lightmanager::addLight(Light *light) {
    unsigned int n;
    switch (light->getType()) {
        case LightType::Directional: {
            n = dirlights;
            dirlights++;
            break;
        }
        case LightType::Spot: {
            n = spotlights;
            spotlights++;
            break;
        }
        case LightType::Point: {
            n = pointlights;
            pointlights++;
            break;
        }
    }
    lights.push_back(new MappedLight(*light, n, map));
    map++; // Be careful not to exceed 32
}

void Lightmanager::addLight(std::vector<Light*> *lights)
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

std::vector<Batch*> Lightmanager::populate(Shader &shadow, Shader &light)
{
    Tree<IDrawable*> *t = new Tree<IDrawable*>();
    Tree<IDrawable*> *tmptree = t;
    for (unsigned int i = 0; i < lights.size(); i++)
    {
        if (i == 0)
        {
            tmptree->addTop(lights);
            tmptree->addTop(drawables);
        }
        tmptree->addBottom(lights[i]);
        tmptree->addBottom(drawables);
    }
    tmptree = t->after;
    std::vector<Batch*> batches;
    std::vector<IDrawable*> tmp;
    while (tmptree != nullptr)
    {
        tmp.insert(tmp.end(), tmptree->entries.begin(), tmptree->entries.end());
        tmptree = tmptree->after;
    }
    Batch* lightbatch = new Batch(0, tmp, &light);
    tmptree = t->before;
    tmp.clear();
    while (tmptree != nullptr)
    {
        tmp.insert(tmp.end(), tmptree->entries.begin(), tmptree->entries.end());
        tmptree = tmptree->before;
    }
    Batch* shadowbatch = new Batch(0, tmp, &shadow);
    batches.push_back(shadowbatch);
    batches.push_back(lightbatch);
    return batches;
}

