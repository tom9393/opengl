#ifndef SHADERNODE
#define SHADERNODE

#include <vector>
#include "drawable.h"
#include "mappedlight.h"
#include "shader.h"
#include "drawable.h"
#include "batch.h"

class ShaderNode {
public:
    ShaderNode(Shader &shader) : shader(&shader) {};
    void addBefore(ShaderNode* node);
    void addAfter(ShaderNode* node);
    void addDrawable(IDrawable* drawable);
    void addDrawable(std::vector<IDrawable*> *drawables);
    void run(std::vector<Batch*> &batches);

private:
    std::vector<IDrawable*> drawables;
    Shader* shader;
    ShaderNode* before = nullptr;
    ShaderNode* after = nullptr;
};

#endif
