#pragma once

#include "Asset.h"

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

        static void GetAssetAsync(UUID handle, std::function<void(Shared<Asset>)> cb)
        {
            GetAssetInternalAsync(handle, cb);
        }

        static bool IsAssetLoaded(UUID handle) { return m_LoadedAssets.find(handle) != m_LoadedAssets.end(); }
        static AssetState GetAssetState(UUID handle) { return m_AssetStates[handle]; }
        static const AssetMetadata& GetMetadata(UUID handle);

        // Names assumed unique; first asset wins
        static UUID GetHandleByName(const std::string& name, AssetType type = AssetType::None);

        // Reverse lookup: find the handle of an already-loaded asset (0 if not loaded).
        static UUID GetHandleForAsset(const Shared<Asset>& asset);

    private:
        static Shared<Asset> GetAssetInternal(UUID handle);
        static void GetAssetInternalAsync(UUID handle, std::function<void(Shared<Asset>)> cb);
        static void LoadAssetRegistry();
        static void SaveAssetRegistry();

        // Bake every loaded model-sourced Scene (.gltf/.fbx/.obj/.assbin) to a sibling
        // .pmscene and repoint its registry entry at it. Runs once at shutdown.
        static void BakeLoadedModelScenes();

    private:
        static inline std::unordered_map<UUID, Shared<Asset>> m_LoadedAssets;
        static inline std::unordered_map<UUID, AssetMetadata> m_AssetRegistry;
        static inline std::unordered_map<UUID, AssetState> m_AssetStates;
        
        static inline AssetMetadata s_NullMetadata; // Returned when handle is invalid

        friend class SceneHierarchyPanel;
        friend class LoadedAssets;
    };

}
