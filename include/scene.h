#ifndef SCENE
#define SCENE

#include "shader.h"
#include <GLFW/glfw3.h>
#include <vector>
class Scene
{
    public:
    Scene();

    void addShader(Shader *shader);
    void addWindow(GLFWwindow *window);
    void draw();
    void predraw();

    private:
    std::vector<Shader*> shaders;
    GLFWwindow *window;

    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 position;
};

#endif
