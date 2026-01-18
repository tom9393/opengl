#include "AssetManager.h"
#include "Asset.h"
#include "SceneImporter.h"
#include "TextureImporter.h"
#include <filesystem>

namespace Engine {

    static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {
            { ".pmblend", AssetType::Scene },
            { ".png", AssetType::Texture2D },
            { ".jpg", AssetType::Texture2D },
            { ".jpeg", AssetType::Texture2D }
    };

    static AssetType AssetTypeFromFilepath(const std::filesystem::path& path)
    {
        std::string ext = path.extension().string();

        if (ext == ".pmblend") return AssetType::Scene;
        if (ext == ".png")     return AssetType::Texture2D;
        if (ext == ".jpg")     return AssetType::Texture2D;
        if (ext == ".jpeg")    return AssetType::Texture2D;

        return AssetType::None;
    }

    bool AssetManager::IsLoaded(UUID id)
    {
        return m_Paths.find(id) != m_Paths.end();
    }

    Shared<Asset> AssetManager::LoadAsset(const std::filesystem::path &path)
    {
        Shared<Asset> asset = GetByType(path);

        if (asset)
        {
            m_Loaded[asset->GetHandle()] = asset;
            m_Paths[asset->GetHandle()] = path;
        }
        return asset;
    };

    Shared<Asset> AssetManager::GetByType(const std::filesystem::path &path)
    {
        switch (AssetTypeFromFilepath(path)) {
            case AssetType::Scene: return SceneImporter::LoadScene(path);
            case AssetType::Texture2D: return TextureImporter::LoadTexture2D(path);
            default: return nullptr;
        }
    }
}
