#pragma once

#include "Asset.h"
#include "../Core/UUID.h"
#include <yaml-cpp/yaml.h>

namespace Engine {

    class AssetManager {
    public:
        static void Init(); // Loads the AssetRegistry.yaml
        static void Shutdown(); // Saves the AssetRegistry.yaml

        // Imports a file, generates a UUID, adds to registry, and returns the UUID
        static UUID ImportAsset(const std::filesystem::path& filepath);
        static UUID CreateMemoryAsset(AssetType type, const std::string &name, Shared<Asset> asset);

        // Core function: gets a loaded asset, or loads it if it isn't in memory
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
        static void SaveAssetMetadata(YAML::Emitter &out, const AssetMetadata &data);
        static void RegisterAssetRecursive(AssetMetadata &data);
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
