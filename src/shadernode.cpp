#include "../include/shadernode.h"

void ShaderNode::addBefore(ShaderNode* node)
{
    before = node;
}

void ShaderNode::addAfter(ShaderNode* node)
{
    after = node;
}

void ShaderNode::addDrawable(IDrawable* drawable)
{
    drawables.push_back(drawable);
}

void ShaderNode::addDrawable(std::vector<IDrawable*> *drawables)
{
    for (unsigned int i = 0; i < drawables->size(); i++) {
        addDrawable(drawables->at(i));
    }
}

void ShaderNode::run(std::vector<Batch*> &batches)
{
    if (before) before->run(batches);
    batches.push_back(new Batch(0, &drawables, shader));
    if (after) after->run(batches);
}

