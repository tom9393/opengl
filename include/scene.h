#ifndef SCENE
#define SCENE

#include "shadernode.h"
#include <GLFW/glfw3.h>

class Scene
{
    public:
    Scene() : batches(new std::vector<Batch*>) {};
    void addWindow(GLFWwindow *window);
    void draw();
    void predraw();
    void setRoot(ShaderNode* root);
    void createBatches();

    private:
    ShaderNode* root;
    GLFWwindow* window;
    std::vector<Batch*>* batches;

    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 position;
};

#endif
