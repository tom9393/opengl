#pragma once

#include "Asset.h"
#include "../Core/UUID.h"
#include <yaml-cpp/yaml.h>

namespace Engine {

    class AssetManager {
    public:
        static void Init();
        static void Shutdown();

        static UUID ImportAsset(const std::filesystem::path& filepath);
        static UUID RegisterAsset(Shared<AssetMetadata> &metadata, Shared<Asset> &asset);

        template<typename T>
        static Shared<T> GetAsset(UUID handle) {
            Shared<Asset> asset = GetAssetInternal(handle);
            return std::static_pointer_cast<T>(asset);
        }

        static bool IsAssetLoaded(UUID handle) { return m_LoadedAssets.find(handle) != m_LoadedAssets.end(); }
        static Shared<AssetMetadata> GetMetadata(UUID handle);

    private:
        static Shared<Asset> GetAssetInternal(UUID handle);
        static Shared<AssetMetadata> LoadAssetMetadata(const YAML::Node &node);
        static void ImportAssetFile(const std::filesystem::path &path);
        static void SaveAssetMetadata(YAML::Emitter &out, const Shared<AssetMetadata> &data);
        static void LoadAssetRegistry();
        static void SaveAssetRegistry();

    private:
        static inline std::unordered_map<UUID, Shared<Asset>> m_LoadedAssets;
        static inline std::unordered_map<UUID, Shared<AssetMetadata>> m_AssetRegistry;
        
        friend class SceneHierarchyPanel;
        friend class LoadedAssets;
    };

}
