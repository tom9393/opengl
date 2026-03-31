#include "AssetManager.h"
#include "Asset.h"
#include "ModelImporter.h"
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
            { ".obj", AssetType::Model }
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

        return AssetType::None;
    }

    bool AssetManager::IsLoaded(const std::string &path)
    {
        return m_Loaded.find(path) != m_Loaded.end();
    }

    Shared<Asset> AssetManager::LoadAsset(const std::filesystem::path &path)
    {
        if (!IsLoaded(path))
        {
            Shared<Asset> asset = AssetManager::GetByType(path);
            if (asset)
            {
                m_Loaded[path] = asset;
            } else {
                std::cout << "Failed to load asset" << std::endl;
            }
        }
        return m_Loaded[path];
    }

    // -- Todo; make the asset system use UUID instead of raw strings --
    // bool AssetManager::IsLoaded(UUID id)
    // {
    //     return m_Paths.find(id) != m_Paths.end();
    // }
    //
    // Shared<Asset> AssetManager::LoadAsset(const std::filesystem::path &path)
    // {
    //     Shared<Asset> asset = GetByType(path);
    //
    //     if (asset)
    //     {
    //         m_Loaded[asset->GetHandle()] = asset;
    //         m_Paths[asset->GetHandle()] = path;
    //     } else {
    //         std::cout << "Failed to load asset" << std::endl;
    //     }
    //     return asset;
    // };

    Shared<Asset> AssetManager::GetByType(const std::filesystem::path &path)
    {
        switch (AssetTypeFromFilepath(path)) {
            case AssetType::Scene: return SceneImporter::LoadScene(path);
            case AssetType::Texture2D: return TextureImporter::LoadTexture2D(path);
            case AssetType::Model: return ModelImporter::LoadModel(path);
            default: return nullptr;
        }
    }
}
