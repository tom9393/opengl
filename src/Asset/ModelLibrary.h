#pragma once

#include "Model.h"
#include "ModelImporter.h"

namespace Engine
{

    class ModelLibrary
    {
        public:

        ModelLibrary(ModelImporter* loader) {};

        Shared<Model> GetModelByPath(std::string &str) {
            if (!m_Models[str])
            {
                Shared<Scene> scene = ModelImporter::LoadModel(str.c_str());
            }
            return m_Models[str];
        };

        private:
        std::unordered_map<std::string, Shared<Model>> m_Models;
    };

};
