#pragma once

#include "Scene.h"
namespace Engine {
    
    class SceneImporter
    {
        public:
        static Shared<Scene> LoadScene(const std::filesystem::path &path);
    };
}
