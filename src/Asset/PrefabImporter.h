#pragma once

#include "Asset.h"

namespace Engine {

    class PrefabImporter
    {
        public:
        static Shared<Asset> LoadPrefab(const AssetMetadata &metadata);
        static void LoadPrefabAsync(const AssetMetadata &metadata, std::function<void(Shared<Asset>)> cb);
    };
}
