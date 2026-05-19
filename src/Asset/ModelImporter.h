#pragma once

#include "Asset.h"

namespace Engine {

    class ModelImporter
    {
        public:
        static Shared<Asset> LoadModel(const AssetMetadata &metadata);
        static void LoadModelAsync(const AssetMetadata metadata, std::function<void(Shared<Asset>)> cb);
    };
}
