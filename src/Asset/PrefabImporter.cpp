#include "PrefabImporter.h"
#include "AssetUploadQueue.h"
#include "Asset/Prefab.h"
#include "Core/Application.h"
#include "Scene/PrefabSerializer.h"

namespace Engine {

    Shared<Asset> PrefabImporter::LoadPrefab(const AssetMetadata &metadata)
    {
        Shared<Prefab> prefab = MakeShared<Prefab>();
        PrefabSerializer serializer(prefab);
        serializer.Deserialize(metadata.FilePath);
        return prefab;
    }

    void PrefabImporter::LoadPrefabAsync(const AssetMetadata &metadata, std::function<void(Shared<Asset>)> cb)
    {
        Application::Get().GetThreadPool().Enqueue([metadata, cb]() {
            Shared<Prefab> prefab = MakeShared<Prefab>();
            PrefabSerializer serializer(prefab);
            serializer.Deserialize(metadata.FilePath);

            AssetUploadQueue::Submit([metadata, prefab, cb]() {
                if (cb) cb(prefab);
            });
        });
    }
}
