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
            { ".fbx", AssetType::Model },
            { ".lua", AssetType::LuaScript }
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
        if (ext == ".lua")     return AssetType::LuaScript;

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
			case AssetType::Model: return "AssetType::Model";
			case AssetType::LuaScript: return "AssetType::LuaScript";
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
		if (assetType == "AssetType::Model") return AssetType::Model;
		if (assetType == "AssetType::LuaScript") return AssetType::LuaScript;

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
                if (metadata.IsValid()) return handle;
            }
        }

        // 2. If not, create new metadata
        AssetMetadata metadata;
        metadata.Handle = UUID();
        metadata.FilePath = filepath.string();
        metadata.Name = filepath.stem().string();
        
        // Determine type based on extension (simplified)
        std::string ext = filepath.extension().string();
        if (ext == ".obj" || ext == ".assbin" || ext == ".gltf" || ext == ".fbx") {
            AssetMetadata mt = AssimpLoader::ScanModel(filepath);
            RegisterAssetRecursive(mt);
            metadata.Handle = mt.Handle;
            metadata.Type = mt.Type;
            metadata.Name = mt.Name;
            metadata.Children = mt.Children;
        }
        else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".ktx" || ext == ".ktx2") metadata.Type = AssetType::Texture2D;
        else if (ext == ".lua") metadata.Type = AssetType::LuaScript;
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

    void AssetManager::RegisterAssetRecursive(AssetMetadata &data)
    {
        m_AssetRegistry[data.Handle] = data;
        if (data.HasChildren())
        {
            for (auto& child : data.Children)
            {
                RegisterAssetRecursive(child);
            }
        }
    }

    AssetMetadata AssetManager::LoadAssetMetadata(const YAML::Node& node)
    {
        AssetMetadata metadata;
        metadata.Handle = node["Handle"].as<uint64_t>();
        metadata.FilePath = node["FilePath"].as<std::string>();
        metadata.Name = node["Name"].as<std::string>();
        metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());
        metadata.Index = node["Index"].as<uint32_t>();

        auto children = node["Children"];
        if (children)
        {
            for (const auto& childNode : children)
            {
                metadata.Children.push_back(LoadAssetMetadata(childNode));
            }
        }

        // Add to the flat registry for quick lookup
        m_AssetRegistry[metadata.Handle] = metadata; 
        return metadata;
    }

    void AssetManager::SaveAssetMetadata(YAML::Emitter& out, const AssetMetadata& metadata)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Handle" << YAML::Value << (uint64_t)metadata.Handle;
        out << YAML::Key << "FilePath" << YAML::Value << metadata.FilePath;
        out << YAML::Key << "Name" << YAML::Value << metadata.Name;
        out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
        out << YAML::Key << "Index" << YAML::Value << metadata.Index;

        if (!metadata.Children.empty())
        {
            out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
            for (const auto& child : metadata.Children)
            {
                SaveAssetMetadata(out, child); // Recursive call
            }
            out << YAML::EndSeq;
        }
        out << YAML::EndMap;
    }

    void AssetManager::SaveAssetRegistry() {
        std::filesystem::path path = "/home/tom/Coding/c++/opengl/assets/AssetRegistry.yaml";
        YAML::Emitter out;
        out << YAML::BeginMap << YAML::Key << "AssetRegistry" << YAML::Value << YAML::BeginSeq;

        for (const auto& [handle, metadata] : m_AssetRegistry)
        {
            // Only save "Root" assets at the top level to avoid duplication
            // (Assuming Models are roots, and Meshes/Materials are children)
            if (metadata.Type == AssetType::Model || metadata.Type == AssetType::Texture2D) 
            {
                SaveAssetMetadata(out, metadata);
            }
        }

        out << YAML::EndSeq << YAML::EndMap;
        std::ofstream fout(path);
        fout << out.c_str();
    }

    void AssetManager::LoadAssetRegistry() {
        std::filesystem::path path = "/home/tom/Coding/c++/opengl/assets/AssetRegistry.yaml";

        if (!std::filesystem::exists(path)) return;

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e)
		{
		}
        auto rootNode = data["AssetRegistry"];
        if (!rootNode) return;

        m_AssetRegistry.clear();
        for (const auto& node : rootNode)
        {
            LoadAssetMetadata(node);
        }
    }

}
