#include "AssetManager.h"
#include "Asset.h"
#include "ModelImporter.h"
#include "AssimpLoader.h"
#include "SceneImporter.h"
#include "TextureImporter.h"
#include <filesystem>
#include <yaml-cpp/yaml.h>

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

    std::string_view AssetTypeToString(AssetType type)
	{
		switch (type)
		{
			case AssetType::None:      return "AssetType::None";
			case AssetType::Scene:     return "AssetType::Scene";
			case AssetType::Mesh: return "AssetType::Mesh";
			case AssetType::Texture2D: return "AssetType::Texture2D";
			case AssetType::Material: return "AssetType::Material";
		}

		return "AssetType::<Invalid>";
	}

	AssetType AssetTypeFromString(std::string_view assetType)
	{
		if (assetType == "AssetType::None")      return AssetType::None;
		if (assetType == "AssetType::Scene")     return AssetType::Scene;
		if (assetType == "AssetType::Mesh") return AssetType::Mesh;
		if (assetType == "AssetType::Texture2D") return AssetType::Texture2D;
		if (assetType == "AssetType::Material") return AssetType::Material;

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
        if (ext == ".obj" || ext == ".assbin") {
            // Create the Model blueprint
            Shared<Model> modelAsset = MakeShared<Model>();
            modelAsset->Handle = UUID();

            // Scan the file and fill the Model's ID vectors
            ModelScanResult hooks = AssimpLoader::ScanModel(filepath, modelAsset);

            // Register all generated Mesh hooks
            for (const auto& meta : hooks.MeshHooks) {
                m_AssetRegistry[meta.Handle] = meta;
            }

            // Register all generated Material hooks
            for (const auto& meta : hooks.MaterialHooks) {
                m_AssetRegistry[meta.Handle] = meta;
            }

            // Register the Model itself
            AssetMetadata modelMetadata;
            modelMetadata.Handle = modelAsset->Handle;
            modelMetadata.Type = AssetType::Model;
            modelMetadata.FilePath = filepath.string();
            m_AssetRegistry[modelAsset->Handle] = modelMetadata;

            // Cache ONLY the lightweight Model struct in memory
            m_LoadedAssets[modelAsset->Handle] = modelAsset;

            SaveAssetRegistry();
            return modelAsset->Handle;
        }
        else if (ext == ".png" || ext == ".jpg" || ext == ".ktx" || ext == ".ktx2") metadata.Type = AssetType::Texture2D;
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
            case AssetType::Mesh:
                newAsset = AssimpLoader::LoadSubMesh(metadata.FilePath, metadata.Index);
                break;
            case AssetType::Texture2D:
                newAsset = TextureImporter::LoadTexture2D(metadata.FilePath);
                break;
            case AssetType::Material:
                newAsset = AssimpLoader::LoadMaterial(metadata.FilePath, metadata.Index);
                break;
            case AssetType::Scene:
                newAsset = SceneImporter::LoadScene(metadata.FilePath);
                break;
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
        std::filesystem::path path = "/home/tom/Coding/c++/opengl/assets/AssetRegistry/yaml";
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e)
		{
		}

		auto rootNode = data["AssetRegistry"];
		if (!rootNode)
			return;

		for (const auto& node : rootNode)
		{
			UUID handle = node["Handle"].as<uint64_t>();
			auto& metadata = m_AssetRegistry[handle];
			metadata.FilePath = node["FilePath"].as<std::string>();
			metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());
		}
    }

    void AssetManager::SaveAssetRegistry() {
        std::filesystem::path path = "/home/tom/Coding/c++/opengl/assets/AssetRegistry/yaml";

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "AssetRegistry" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto&[handle, metadata] : m_AssetRegistry)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				std::string filepathStr = metadata.FilePath.c_str();
				out << YAML::Key << "FilePath" << YAML::Value << filepathStr;
				out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
				out << YAML::Key << "Index" << YAML::Value << metadata.Index;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap; // Root
		}

		std::ofstream fout(path);
		fout << out.c_str();
    }

}
