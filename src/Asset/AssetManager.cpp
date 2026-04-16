#include "AssetManager.h"
#include "Asset.h"
#include "ModelImporter.h"
#include "AssimpLoader.h"
#include "SceneImporter.h"
#include "TextureImporter.h"
#include "ScriptImporter.h"
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
        if (ext == ".gltf")    return AssetType::Model;
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
        for (const auto& [handle, metadata] : m_AssetRegistry) {
            if (metadata->FilePath == filepath.string()) {
                if (metadata->IsValid()) return handle;
            }
        }

        Shared<AssetMetadata> metadata;

        switch (AssetTypeFromFilepath(filepath))
        {
            case AssetType::Scene: metadata = SceneImporter::ImportScene(filepath); break;
            case AssetType::Model: metadata = AssimpLoader::ImportModel(filepath); break;
            case AssetType::Texture2D: metadata = TextureImporter::ImportTexture2D(filepath); break;
            case AssetType::LuaScript: metadata = ScriptImporter::ImportLuaScript(filepath); break;
        }

        m_AssetRegistry[metadata->Handle] = metadata;
        
        SaveAssetRegistry();
        return metadata->Handle;
    }

    UUID AssetManager::RegisterAsset(Shared<AssetMetadata> &metadata, Shared<Asset> &asset)
    {
        m_AssetRegistry[metadata->Handle] = metadata;
        m_LoadedAssets[metadata->Handle] = asset;
        return metadata->Handle;
    }

    Shared<Asset> AssetManager::GetAssetInternal(UUID handle) {
        if (handle == 0) return nullptr;

        if (IsAssetLoaded(handle)) {
            return m_LoadedAssets[handle];
        }

        Shared<AssetMetadata> metadata = GetMetadata(handle);

        Shared<Asset> newAsset = nullptr;
        switch (metadata->Type) {
            case AssetType::Mesh: newAsset = AssimpLoader::LoadSubMesh(metadata); break;
            case AssetType::Texture2D: newAsset = TextureImporter::LoadTexture2D(metadata); break;
            case AssetType::Material: newAsset = AssimpLoader::LoadMaterial(metadata); break;
            case AssetType::Scene: newAsset = SceneImporter::LoadScene(metadata); break;
            case AssetType::Model: newAsset = AssimpLoader::LoadSceneFromAsset(metadata); break;
        }

        if (newAsset) {
            newAsset->Handle = handle;
            m_LoadedAssets[handle] = newAsset;
        }

        return newAsset;
    }

    Shared<AssetMetadata> AssetManager::GetMetadata(UUID handle) {
        auto it = m_AssetRegistry.find(handle);
        if (it != m_AssetRegistry.end())
            return it->second;
        return nullptr;
    }

    Shared<AssetMetadata> AssetManager::LoadAssetMetadata(const YAML::Node& node)
    {
        std::string typeString = node["Type"].as<std::string>();
        AssetType type = AssetTypeFromString(typeString);

        Shared<AssetMetadata> metadata;

        // Instantiate the correct derived class
        switch (type)
        {
            case AssetType::Mesh:     metadata = MakeShared<MeshAssetMetadata>(); break;
            case AssetType::Material: metadata = MakeShared<MaterialAssetMetadata>(); break;
            case AssetType::Model:    // Fallthrough...
            case AssetType::Scene:    // Fallthrough...
            default:                  metadata = MakeShared<AssetMetadata>(); break;
        }

        // The overridden virtual method takes care of the exact fields
        metadata->LoadYAMLNode(node);

        m_AssetRegistry[metadata->Handle] = metadata; 
        
        if (metadata->Type == AssetType::Model) {
            ImportAssetFile(metadata->FilePath);
        }
        
        return metadata;
    }

    void AssetManager::SaveAssetMetadata(YAML::Emitter& out, const Shared<AssetMetadata> &metadata)
    {
        if (metadata->Type == AssetType::Mesh || metadata->Type == AssetType::Material) return;
        out << YAML::BeginMap;
        metadata->SaveYAMLNode(out);
        out << YAML::EndMap;
        if (metadata->Type == AssetType::Model) ImportAssetFile(metadata->FilePath);
    }

    void AssetManager::SaveAssetRegistry() 
    {
        std::filesystem::path path = "/home/tom/Coding/c++/opengl/assets/AssetRegistry.yaml";
        YAML::Emitter out;
        out << YAML::BeginMap << YAML::Key << "AssetRegistry" << YAML::Value << YAML::BeginSeq;

        for (const auto& [handle, metadata] : m_AssetRegistry)
        {
            SaveAssetMetadata(out, metadata);
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
            LoadAssetMetadata(node);
        }
    }

    void AssetManager::ImportAssetFile(const std::filesystem::path& assetFilePath)
    {
        if (!std::filesystem::exists(assetFilePath)) return;

        YAML::Node data;
        try {
            data = YAML::LoadFile(assetFilePath.string());
        } catch (YAML::ParserException e) {
            std::cout << "ERROR::YAML:: Failed to load .asset file: " << assetFilePath << "\n";
            return;
        }

        auto modelNode = data["Model"];
        if (!modelNode) return;

        // The path to the actual geometry file (.obj, .gltf)
        std::string sourceFilePath = modelNode["SourceFilePath"].as<std::string>();

        // 1. Register the Model Asset
        /*
        AssetMetadata modelMeta;
        modelMeta.Handle = modelNode["Handle"].as<uint64_t>();
        modelMeta.FilePath = sourceFilePath; 
        modelMeta.Name = modelNode["Name"].as<std::string>();
        modelMeta.Type = AssetType::Model; 
        modelMeta.Index = 0;
        m_AssetRegistry[modelMeta.Handle] = modelMeta;
        */

        // Define a recursive lambda to traverse the node tree
        std::function<void(const YAML::Node&)> processNode = [&](const YAML::Node& node) 
        {
            if (!node) return;

            // 2. Iterate through Meshes attached to THIS specific node
            auto meshesNode = node["Meshes"];
            if (meshesNode) 
            {
                for (auto meshNode : meshesNode) 
                {
                    // Register the Submesh Asset
                    AssetMetadata meshMeta;
                    meshMeta.Handle = meshNode["Handle"].as<uint64_t>();
                    meshMeta.FilePath = sourceFilePath;
                    meshMeta.Name = meshNode["Name"].as<std::string>();
                    meshMeta.Type = AssetType::Mesh; 
                    meshMeta.Index = meshNode["Index"].as<uint32_t>();
                    m_AssetRegistry[meshMeta.Handle] = meshMeta;

                    // Register the Material Asset
                    auto materialNode = meshNode["Material"];
                    if (materialNode) 
                    {
                        AssetMetadata matMeta;
                        matMeta.Handle = materialNode["Handle"].as<uint64_t>();
                        matMeta.FilePath = sourceFilePath;
                        matMeta.Name = materialNode["Name"].as<std::string>();
                        matMeta.Type = AssetType::Material; 
                        matMeta.Index = materialNode["Index"].as<uint32_t>();
                        m_AssetRegistry[matMeta.Handle] = matMeta;
                    }
                }
            }

            // 3. Process any child nodes recursively
            auto childrenNode = node["Children"];
            if (childrenNode) 
            {
                for (auto childNode : childrenNode) 
                {
                    processNode(childNode);
                }
            }
        };

        // Start the recursive processing from the root of the hierarchy
        auto hierarchyNode = modelNode["Hierarchy"];
        if (hierarchyNode) 
        {
            processNode(hierarchyNode);
        }
        
        std::cout << "Successfully imported assets from: " << assetFilePath.filename().string() << std::endl;
    }

}
