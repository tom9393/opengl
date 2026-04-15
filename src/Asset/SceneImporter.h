#pragma once

#include "Scene.h"
namespace Engine {
    
    class SceneImporter
    {
        public:
        static Shared<Scene> LoadScene(const AssetMetadata &metadata);

        static AssetMetadata ImportScene(const std::filesystem::path &path);
    };
}
