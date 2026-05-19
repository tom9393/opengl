#include "ModelImporter.h"
#include "AssimpLoader.h"

namespace Engine {

    Shared<Asset> ModelImporter::LoadModel(const AssetMetadata &metadata)
    {
        return AssimpLoader::LoadModel(metadata);
    }

    void ModelImporter::LoadModelAsync(const AssetMetadata metadata, std::function<void(Shared<Asset>)> cb)
    {
        AssimpLoader::LoadModelAsync(metadata, cb);
    }
}
