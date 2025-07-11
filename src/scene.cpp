#include "../include/scene.h"
#include "../include/Input.h"

void Scene::draw()
{
    predraw();
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    for (unsigned int i = 0; i < shaders.size(); i++)
    {
        Lighting *shader = shaders[i];
        shader->Enable();

        shader->setMat4("view", view);
        shader->setMat4("projection", projection);
        shader->setVec3("viewPos", position);

        shader->setup();
        shader->draw();
        shader->cleanup();
    }
    glfwSwapBuffers(window);
    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Scene::addShader(Lighting* shader)
{
    shader->lightmanager = this->lightmanager;
    shaders.push_back(shader);
}
void Scene::addWindow(GLFWwindow* win)
{
    window = win;
}

void Scene::addDrawable(IDrawable* drawable) {
    for (unsigned int i = 0; i < shaders.size(); i++) {
        Lighting* shader = shaders[i];
        shader->addDrawable(drawable);
    }
}

void Scene::addLight(ILight* light) {
    lightmanager->addLight(light);
}

void Scene::predraw()
{
    InputHandler::process(window);
    Camera camera = InputHandler::getCamera();

    view = camera.getViewMatrix();
    projection = glm::perspective(glm::radians(60.0f), (float)1600 / (float)1600, 0.1f, 100.0f);
    position = camera.Position;
}
