#ifndef SCENE
#define SCENE

#include "lighting.h"
#include "shader.h"
#include <GLFW/glfw3.h>
#include <vector>
#include "../include/obj.h"
#include "../include/light.h"
#include "shadowmap.h"

class Scene
{
    public:
    Scene();

    void addShader(Lighting *shader);
    void addWindow(GLFWwindow *window);
    void addObject(Obj *object);
    void addLight(ILight *light);
    void draw();
    void predraw();

    private:
    std::vector<Lighting*> shaders;
    GLFWwindow *window;

    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 position;

    unsigned int spotlights;
    unsigned int pointlights;
    unsigned int dirlights;
};

#endif
