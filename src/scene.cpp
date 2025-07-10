#include "../include/scene.h"
#include "../include/Input.h"

Scene::Scene()
{}

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
    shaders.push_back(shader);
}
void Scene::addWindow(GLFWwindow* win)
{
    window = win;
}

void Scene::addObject(Obj* object) {
    for (unsigned int i = 0; i < shaders.size(); i++) {
        Lighting* shader = shaders[i];
        shader->addObj(object);
    }
}

void Scene::addLight(ILight* light) {

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

    for (unsigned int i = 0; i < shaders.size(); i++) {
        Lighting* shader = shaders[i];
        shader->addLight(l);
    }
}

void Scene::predraw()
{
    InputHandler::process(window);
    Camera camera = InputHandler::getCamera();

    view = camera.getViewMatrix();
    projection = glm::perspective(glm::radians(60.0f), (float)1600 / (float)1600, 0.1f, 100.0f);
    position = camera.Position;
}
