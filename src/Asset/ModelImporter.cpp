#include "ModelImporter.h"
#include "AssimpLoader.h"
#include "SceneBuilder.h"
#include "AssetUploadQueue.h"
#include "Core/Application.h"

namespace Engine {

    Shared<Asset> ModelImporter::LoadModel(const AssetMetadata &metadata)
    {
        std::optional<ParsedScene> scene = AssimpLoader::LoadScene(metadata);
        if (!scene)
            return nullptr;

        return SceneBuilder::BuildPrefab(metadata, *scene);
    }

    void ModelImporter::LoadModelAsync(const AssetMetadata metadata, std::function<void(Shared<Asset>)> cb)
    {
        Application::Get().GetThreadPool().Enqueue([metadata, cb]() {
            std::optional<ParsedScene> scene = AssimpLoader::LoadScene(metadata);
            if (!scene)
                return;

            AssetUploadQueue::Submit([metadata, scene = std::move(*scene), cb]() {
                Shared<Prefab> prefab = SceneBuilder::BuildPrefab(metadata, scene);
                if (cb) cb(prefab);
            });
        });
    }
}
