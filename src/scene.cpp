#include "../include/scene.h"

void Scene::draw()
{
    for (unsigned int i = 0; i < batches->size(); i++)
    {
        Batch* batch = batches->at(i);
        batch->render();
    }
}

void Scene::setRoot(ShaderNode* root) {
    this->root = root;
}

void Scene::createBatches()
{
    root->run(*batches);
}
