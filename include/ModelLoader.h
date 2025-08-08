#pragma once

#include "model.h"

class ModelLoader
{
    public:
    virtual std::shared_ptr<Model> Load(const char* path) = 0;
};
