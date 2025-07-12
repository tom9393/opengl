#include "../include/scene.h"

void Scene::draw()
{
    predraw();
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    for (unsigned int i = 0; i < batches->size(); i++)
    {
        Batch* batch = batches->at(i);
        batch->shader->Enable();
        batch->shader->setMat4("view", view);
        batch->shader->setMat4("projection", projection);
        batch->shader->setVec3("viewPos", position);
        batch->render();
    }
    glfwSwapBuffers(window->getWindow());
    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Scene::addWindow(Window* win)
{
    window = win;
}

void Scene::predraw()
{
    window->updateDelta();

    Camera* camera = window->getCamera();

    view = camera->getViewMatrix();
    projection = glm::perspective(glm::radians(60.0f), (float)1600 / (float)1600, 0.1f, 100.0f);
    position = camera->Position;
}

void Scene::setRoot(ShaderNode* root) {
    this->root = root;
}

void Scene::createBatches()
{
    root->run(*batches);
}
