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
        static UUID CreateMemoryAsset(AssetType type, const std::string &name, Shared<Asset> asset);

        template<typename T>
        static Shared<T> GetAsset(UUID handle) {
            Shared<Asset> asset = GetAssetInternal(handle);
            return std::static_pointer_cast<T>(asset);
        }

        static bool IsAssetLoaded(UUID handle) { return m_LoadedAssets.find(handle) != m_LoadedAssets.end(); }
        static const AssetMetadata& GetMetadata(UUID handle);

    private:
        static Shared<Asset> GetAssetInternal(UUID handle);
        static AssetMetadata LoadAssetMetadata(const YAML::Node &node);
        static void ImportAssetFile(const std::filesystem::path &path);
        static void SaveAssetMetadata(YAML::Emitter &out, const AssetMetadata &data);
        static void LoadAssetRegistry();
        static void SaveAssetRegistry();

    private:
        static inline std::unordered_map<UUID, Shared<Asset>> m_LoadedAssets;
        static inline std::unordered_map<UUID, AssetMetadata> m_AssetRegistry;
        
        static inline AssetMetadata s_NullMetadata; // Returned when handle is invalid

        friend class SceneHierarchyPanel;
        friend class LoadedAssets;
    };

}
