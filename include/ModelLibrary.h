#pragma once

#include "model.h"
#include "ModelLoader.h"

class ModelLibrary
{
    public:

    ModelLibrary(ModelLoader* loader) : m_Loader(loader) {};

    Shared<Model> GetModelByPath(std::string &str) {
        if (!m_Models[str])
        {
            m_Models[str] = m_Loader->Load(str.c_str());
        }
        return m_Models[str];
    };

    private:
    std::unordered_map<std::string, Shared<Model>> m_Models;
    ModelLoader* m_Loader;
};
