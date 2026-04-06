#include "AssetManager.h"
#include "Asset.h"
#include "ModelImporter.h"
#include "AssimpLoader.h"
#include "SceneImporter.h"
#include "TextureImporter.h"
#include <filesystem>

namespace Engine {

    static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {
            { ".pmblend", AssetType::Scene },
            { ".png", AssetType::Texture2D },
            { ".jpg", AssetType::Texture2D },
            { ".jpeg", AssetType::Texture2D },
            { ".ktx", AssetType::Texture2D },
            { ".ktx2", AssetType::Texture2D },
            { ".assbin", AssetType::Model },
            { ".obj", AssetType::Model },
            { ".fbx", AssetType::Model }
    };

    static AssetType AssetTypeFromFilepath(const std::filesystem::path& path)
    {
        std::string ext = path.extension().string();

        if (ext == ".pmblend") return AssetType::Scene;
        if (ext == ".png")     return AssetType::Texture2D;
        if (ext == ".jpg")     return AssetType::Texture2D;
        if (ext == ".jpeg")    return AssetType::Texture2D;
        if (ext == ".ktx")     return AssetType::Texture2D;
        if (ext == ".ktx2")    return AssetType::Texture2D;
        if (ext == ".assbin")  return AssetType::Model;
        if (ext == ".obj")     return AssetType::Model;
        if (ext == ".fbx")     return AssetType::Model;

        return AssetType::None;
    }

    void AssetManager::Init() {
        LoadAssetRegistry();
    }

    void AssetManager::Shutdown() {
        SaveAssetRegistry();
    }

    UUID AssetManager::ImportAsset(const std::filesystem::path& filepath) {
        // 1. Check if we already have this file in the registry
        for (const auto& [handle, metadata] : m_AssetRegistry) {
            if (metadata.FilePath == filepath.string()) {
                return handle;
            }
        }

        // 2. If not, create new metadata
        AssetMetadata metadata;
        metadata.Handle = UUID();
        metadata.FilePath = filepath.string();
        
        // Determine type based on extension (simplified)
        std::string ext = filepath.extension().string();
        if (ext == ".obj" || ext == ".assbin") metadata.Type = AssetType::Model; // Parse model here, fill metadata with all the stuff
        else if (ext == ".png" || ext == ".jpg") metadata.Type = AssetType::Texture2D;
        else metadata.Type = AssetType::None;

        m_AssetRegistry[metadata.Handle] = metadata;
        
        SaveAssetRegistry(); // Optional: Save immediately on import
        return metadata.Handle;
    }

    Shared<Asset> AssetManager::GetAssetInternal(UUID handle) {
        if (handle == 0) return nullptr;

        // 1. Is it already loaded in memory?
        if (IsAssetLoaded(handle)) {
            return m_LoadedAssets[handle];
        }

        // 2. Not loaded. Do we know about it in the registry?
        const AssetMetadata& metadata = GetMetadata(handle);
        if (!metadata.IsValid()) {
            // Unregistered asset requested
            return nullptr; 
        }

        // 3. Load from disk based on metadata
        Shared<Asset> newAsset = nullptr;
        
        switch (metadata.Type) {
            case AssetType::Model:
                newAsset = AssimpLoader::Load(metadata.FilePath); // Then return the correct one here
                break;
            case AssetType::Texture2D:
                // newAsset = MakeShared<Texture>(metadata.FilePath);
                break;
            // ... handle other types
        }

        if (newAsset) {
            newAsset->Handle = handle;
            m_LoadedAssets[handle] = newAsset;
        }

        return newAsset;
    }

    const AssetMetadata& AssetManager::GetMetadata(UUID handle) {
        auto it = m_AssetRegistry.find(handle);
        if (it != m_AssetRegistry.end())
            return it->second;
        return s_NullMetadata;
    }

    void AssetManager::LoadAssetRegistry() {
        // Use YAML-CPP to load a generic "AssetRegistry.yaml" from your project root
        // and populate m_AssetRegistry.
    }

    void AssetManager::SaveAssetRegistry() {
        // Use YAML-CPP to write m_AssetRegistry to "AssetRegistry.yaml".
    }

}
