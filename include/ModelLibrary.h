#pragma once

#include "Model.h"
#include "ModelLoader.h"

namespace Engine
{

    class ModelLibrary
    {
        public:

        ModelLibrary(ModelLoader* loader) : m_Loader(loader) {};

        Shared<Model> GetModelByPath(std::string &str, int id) {
            if (!m_Models[str])
            {
                m_Models[str] = m_Loader->Load(str.c_str(), id);
            }
            return m_Models[str];
        };

        private:
        std::unordered_map<std::string, Shared<Model>> m_Models;
        ModelLoader* m_Loader;
    };

};
