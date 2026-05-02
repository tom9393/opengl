#include "../pch.h"
#include "AssetManager.h"
#include "Asset.h"
#include "AssimpLoader.h"
#include "SceneImporter.h"
#include "TextureImporter.h"
#include "ScriptImporter.h"
#include <yaml-cpp/yaml.h>

namespace Engine {

    static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {
            { ".pmblend", AssetType::Scene },
            { ".png", AssetType::Texture2D },
            { ".jpg", AssetType::Texture2D },
            { ".jpeg", AssetType::Texture2D },
            { ".ktx", AssetType::Texture2D },
            { ".ktx2", AssetType::Texture2D },
            { ".assbin", AssetType::Prefab },
            { ".obj", AssetType::Prefab },
            { ".fbx", AssetType::Prefab },
            { ".lua", AssetType::LuaScript }
    };

    void AssetManager::Init() {
        LoadAssetRegistry();
    }

    void AssetManager::Shutdown() {
        SaveAssetRegistry();
    }

    UUID AssetManager::ImportAsset(const std::filesystem::path& filepath) {
        for (const auto& [handle, metadata] : m_AssetRegistry) {
            if (metadata.FilePath == filepath.string()) {
                if (metadata.IsValid()) return handle;
            }
        }

        AssetMetadata metadata;
        metadata.Handle = UUID();
        metadata.FilePath = filepath.string();
        metadata.Name = filepath.stem().string();
        
        std::string ext = filepath.extension().string();
        if (ext == ".obj" || ext == ".assbin" || ext == ".gltf" || ext == ".fbx") metadata.Type = AssetType::Prefab;
        else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".ktx" || ext == ".ktx2") metadata.Type = AssetType::Texture2D;
        else if (ext == ".lua") metadata.Type = AssetType::LuaScript;
        else metadata.Type = AssetType::None;

        m_AssetRegistry[metadata.Handle] = metadata;
        
        SaveAssetRegistry();
        return metadata.Handle;
    }

    UUID AssetManager::RegisterAsset(const AssetMetadata &metadata, Shared<Asset> &asset)
    {
        asset->Handle = metadata.Handle;
        m_AssetRegistry[metadata.Handle] = metadata;
        m_LoadedAssets[metadata.Handle] = asset;
        m_AssetStates[metadata.Handle] = AssetState::Ready;
        return metadata.Handle;
    }

    Shared<Asset> AssetManager::GetAssetInternal(UUID handle) {
        if (handle == 0) return nullptr;

        if (IsAssetLoaded(handle)) {
            return m_LoadedAssets[handle];
        }

        const AssetMetadata& metadata = GetMetadata(handle);

        Shared<Asset> newAsset = nullptr;
        switch (metadata.Type) {
            case AssetType::Texture2D: newAsset = TextureImporter::LoadTexture2D(metadata); break;
            case AssetType::Scene: newAsset = SceneImporter::LoadScene(metadata); break;
            case AssetType::Prefab: newAsset = AssimpLoader::LoadModel(metadata); break;
        }

        if (newAsset) {
            newAsset->Handle = handle;
            m_LoadedAssets[handle] = newAsset;
        }

        return newAsset;
    }

    void AssetManager::GetAssetInternalAsync(UUID handle, std::function<void(Shared<Asset>)> cb) {
        if (handle == 0) return;

        if (m_AssetStates[handle] == AssetState::Loading) return;

        if (m_AssetStates[handle] == AssetState::Ready && IsAssetLoaded(handle)) {
            cb(m_LoadedAssets[handle]);
            return;
        }

        const AssetMetadata& metadata = GetMetadata(handle);
        m_AssetStates[handle] = AssetState::Loading;

        switch (metadata.Type) {
            case AssetType::Texture2D: TextureImporter::LoadTexture2DAsync(metadata, cb); break;
            case AssetType::Scene: SceneImporter::LoadSceneAsync(metadata, cb); break;
            case AssetType::Prefab: AssimpLoader::LoadModelAsync(metadata, cb); break;
        }
    }

    const AssetMetadata& AssetManager::GetMetadata(UUID handle) {
        auto it = m_AssetRegistry.find(handle);
        if (it != m_AssetRegistry.end())
            return it->second;
        return s_NullMetadata;
    }

    void AssetManager::SaveAssetRegistry() 
    {
        std::filesystem::path path = "/home/tom/Coding/c++/opengl/assets/AssetRegistry.yaml";
        YAML::Emitter out;
        out << YAML::BeginMap << YAML::Key << "AssetRegistry" << YAML::Value << YAML::BeginSeq;

        for (const auto& [handle, metadata] : m_AssetRegistry)
        {
            if (metadata.IsMemoryAsset) continue;

            out << YAML::BeginMap;
            out << YAML::Key << "Handle" << YAML::Value << (uint64_t)metadata.Handle;
            out << YAML::Key << "FilePath" << YAML::Value << metadata.FilePath;
            out << YAML::Key << "Name" << YAML::Value << metadata.Name;
            out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
            out << YAML::EndMap;
        }

        out << YAML::EndSeq << YAML::EndMap;
        std::ofstream fout(path);
        fout << out.c_str();
    }

    void AssetManager::LoadAssetRegistry() 
    {
        std::filesystem::path path = "/home/tom/Coding/c++/opengl/assets/AssetRegistry.yaml";

        if (!std::filesystem::exists(path)) return;

        YAML::Node data;
        try {
            data = YAML::LoadFile(path.string());
        } catch (YAML::ParserException e) {
            std::cout << "Failed to load Asset Registry!\n";
            return;
        }

        auto rootNode = data["AssetRegistry"];
        if (!rootNode) return;

        m_AssetRegistry.clear();
        for (const auto& node : rootNode)
        {
            AssetMetadata metadata;
            metadata.Handle = node["Handle"].as<uint64_t>();
            metadata.FilePath = node["FilePath"].as<std::string>();
            metadata.Name = node["Name"].as<std::string>();
            metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());
            m_AssetRegistry[metadata.Handle] = metadata; 
        }
    }

}
