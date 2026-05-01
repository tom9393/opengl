#pragma once

#include "Asset.h"
#include "../Core/UUID.h"
#include <yaml-cpp/yaml.h>

namespace Engine {

    enum class AssetState {
        Unloaded = 0,
        Loading,
        Ready,
        Failed
    };

    class AssetManager {
    public:
        static void Init();
        static void Shutdown();

        static UUID ImportAsset(const std::filesystem::path& filepath);
        static UUID RegisterAsset(const AssetMetadata &metadata, Shared<Asset> &asset);

        template<typename T>
        static Shared<T> GetAsset(UUID handle) {
            Shared<Asset> asset = GetAssetInternal(handle);
            return std::static_pointer_cast<T>(asset);
        }

        template <typename T>
        static Shared<T> GetAssetAsync(UUID handle)
        {
            Shared<Asset> asset = GetAssetInternalAsync(handle);
            return std::static_pointer_cast<T>(asset);
        }

        static bool IsAssetLoaded(UUID handle) { return m_LoadedAssets.find(handle) != m_LoadedAssets.end(); }
        static AssetState GetAssetState(UUID handle) { return m_AssetStates[handle]; }
        static const AssetMetadata& GetMetadata(UUID handle);

    private:
        static Shared<Asset> GetAssetInternal(UUID handle);
        static Shared<Asset> GetAssetInternalAsync(UUID handle);
        static void LoadAssetRegistry();
        static void SaveAssetRegistry();

    private:
        static inline std::unordered_map<UUID, Shared<Asset>> m_LoadedAssets;
        static inline std::unordered_map<UUID, AssetMetadata> m_AssetRegistry;
        static inline std::unordered_map<UUID, AssetState> m_AssetStates;
        
        static inline AssetMetadata s_NullMetadata; // Returned when handle is invalid

        friend class SceneHierarchyPanel;
        friend class LoadedAssets;
    };

}
