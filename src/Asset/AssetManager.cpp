#include <yaml-cpp/yaml.h>

#include "pch.h"
#include "AssetManager.h"
#include "AssimpLoader.h"
#include "Asset.h"
#include "SceneImporter.h"
#include "PrefabImporter.h"
#include "ScriptImporter.h"
#include "TextureImporter.h"
#include "AudioImporter.h"
#include "PmSceneSerializer.h"
#include "Prefab.h"

namespace Engine {

    static const char* s_AssetRegistryPath = "../assets/AssetRegistry.yaml";

    void AssetManager::Init() {
        LoadAssetRegistry();
    }

    void AssetManager::Shutdown() {
        BakeLoadedModelScenes();
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
        metadata.Type = AssetTypeFromFilepath(filepath);

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
            case AssetType::Prefab: newAsset = PrefabImporter::LoadPrefab(metadata); break;
            case AssetType::Audio: newAsset = AudioImporter::LoadAudio(metadata); break;
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
            case AssetType::Prefab: PrefabImporter::LoadPrefabAsync(metadata, cb); break;
            case AssetType::Audio: AudioImporter::LoadAudioAsync(metadata, cb); break;
        }
    }

    const AssetMetadata& AssetManager::GetMetadata(UUID handle) {
        auto it = m_AssetRegistry.find(handle);
        if (it != m_AssetRegistry.end())
            return it->second;
        return s_NullMetadata;
    }

    UUID AssetManager::GetHandleByName(const std::string& name, AssetType type) {
        for (const auto& [handle, metadata] : m_AssetRegistry)
        {
            if (metadata.Name != name) continue;
            if (type != AssetType::None && metadata.Type != type) continue;
            return handle;
        }
        return 0;
    }

    UUID AssetManager::GetHandleForAsset(const Shared<Asset>& asset)
    {
        if (!asset) return 0;
        for (const auto& [handle, loaded] : m_LoadedAssets)
            if (loaded == asset) return handle;
        return 0;
    }

    void AssetManager::BakeLoadedModelScenes()
    {
        std::vector<UUID> toBake;
        for (const auto& [handle, metadata] : m_AssetRegistry)
        {
            if (metadata.Type != AssetType::Scene) continue;
            std::string ext = std::filesystem::path(metadata.FilePath).extension().string();
            bool isModel = (ext == ".gltf" || ext == ".fbx" || ext == ".obj" || ext == ".assbin");
            if (!isModel) continue;
            if (!IsAssetLoaded(handle)) continue;
            toBake.push_back(handle);
        }

        // Todo: do not bake non-changes scenes
        for (UUID handle : toBake)
        {
            AssetMetadata& metadata = m_AssetRegistry[handle];
            Shared<Prefab> prefab = GetAsset<Prefab>(handle);
            if (!prefab) continue;

            std::filesystem::path pmscenePath = std::filesystem::path(metadata.FilePath).replace_extension(".pmscene");
            PmSceneSerializer::Serialize(prefab, pmscenePath.string());

            metadata.FilePath = pmscenePath.string();
            std::cout << "Baked scene to " << metadata.FilePath << std::endl;
        }
    }

    void AssetManager::SaveAssetRegistry()
    {
        std::filesystem::path path = s_AssetRegistryPath;
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
        std::filesystem::path path = s_AssetRegistryPath;

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
