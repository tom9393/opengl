#pragma once

#include "Scene.h"
namespace Engine {
    
    class SceneImporter
    {
        public:
        static Shared<Scene> LoadScene(const AssetMetadata &metadata);

        static Shared<AssetMetadata> ImportScene(const std::filesystem::path &path);
    };
}
