#pragma once

#include "Model.h"

namespace Engine {

    class ModelImporter
    {
        public:
        static Shared<Model> LoadModel(const std::filesystem::path &path);
    };
}
