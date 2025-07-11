#ifndef SCENE
#define SCENE

#include "lighting.h"
#include "lightmanager.h"
#include "shader.h"
#include <GLFW/glfw3.h>
#include <vector>
#include "../include/obj.h"
#include "../include/light.h"
#include "shadowmap.h"

class Scene
{
    public:
    Scene() : lightmanager(new Lightmanager) {}

    void addShader(Lighting *shader);
    void addWindow(GLFWwindow *window);
    void addDrawable(IDrawable *drawable);
    void addLight(ILight *light);
    void draw();
    void predraw();

    private:
    std::vector<Lighting*> shaders;
    Lightmanager* lightmanager;
    GLFWwindow *window;

    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 position;
};

#endif
