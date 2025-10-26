#pragma once

#include "model.h"

class ModelLoader
{
    public:
    virtual std::shared_ptr<Model> Load(const char* path, int id) = 0;
};
