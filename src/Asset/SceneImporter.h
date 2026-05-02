#pragma once

#include "Scene.h"
namespace Engine {
    
    class SceneImporter
    {
        public:
        static Shared<Scene> LoadScene(const AssetMetadata &metadata);
        static void LoadSceneAsync(const AssetMetadata &metadata, std::function<void(Shared<Asset>)> cb);

        static Shared<AssetMetadata> ImportScene(const std::filesystem::path &path);
    };
}
