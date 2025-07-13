#ifndef SCENE
#define SCENE

#include "shadernode.h"
#include <GLFW/glfw3.h>

class Scene
{
    public:
    Scene() : batches(new std::vector<Batch*>) {};
    void draw();
    void setRoot(ShaderNode* root);
    void createBatches();

    private:
    ShaderNode* root;
    std::vector<Batch*>* batches;
};

#endif
