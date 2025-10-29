#pragma once

#include "Model.h"

class ModelLoader
{
    public:
    virtual Shared<Model> Load(const char* path, int id) = 0;
};
