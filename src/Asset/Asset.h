#pragma once

#include "../Core/UUID.h"

namespace Engine {

    enum class AssetType
    {
        None = 0,
        Texture2D,
        Texture3D,
        Scene,
        Model,
        Mesh,
        Material,
        LuaScript
    };

    struct AssetMetadata {
        UUID Handle = 0;
        AssetType Type = AssetType::None;
        bool IsMemoryAsset = false;
        std::string FilePath;
        std::string Name = "";

        virtual ~AssetMetadata() = default;

        virtual void LoadYAMLNode(const YAML::Node& node)
        {
            Handle = node["Handle"].as<uint64_t>();
            FilePath = node["FilePath"].as<std::string>();
            Name = node["Name"].as<std::string>();
            Type = AssetTypeFromString(node["Type"].as<std::string>());
        }

        virtual void SaveYAMLNode(YAML::Emitter& out) const
        {
            out << YAML::Key << "Handle" << YAML::Value << (uint64_t)Handle;
            out << YAML::Key << "FilePath" << YAML::Value << FilePath;
            out << YAML::Key << "Name" << YAML::Value << Name;
            out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(Type);
        }
        
        bool IsValid() const { return Handle != 0; }
    };

    struct NodeAssetMetadata : public AssetMetadata
    {
        glm::vec3 Translation = glm::vec3(0.0f);
        glm::vec3 Rotation = glm::vec3(0.0f);
        glm::vec3 Scale = glm::vec3(1.0f);

        virtual void LoadYAMLNode(const YAML::Node &node) override
        {
            AssetMetadata::LoadYAMLNode(node);

            if (auto transformNode = node["Transform"]) 
            {
                Translation = transformNode["Translation"].as<glm::vec3>();
                Rotation = transformNode["Rotation"].as<glm::vec3>();
                Scale = transformNode["Scale"].as<glm::vec3>();
            }
        }

        virtual void SaveYAMLNode(YAML::Emitter &out) const override
        {
            AssetMetadata::SaveYAMLNode(out);

            out << YAML::Key << "Transform" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "Translation" << YAML::Value << Translation;
            out << YAML::Key << "Rotation" << YAML::Value << Rotation;
            out << YAML::Key << "Scale" << YAML::Value << Scale;
            out << YAML::EndMap;
        }
    };

    struct MeshAssetMetadata : public AssetMetadata
    {
        uint32_t Index = 0;

        virtual void LoadYAMLNode(const YAML::Node& node) override
        {
            AssetMetadata::LoadYAMLNode(node);
            
            if (node["Index"])
                Index = node["Index"].as<uint32_t>();

        }

        virtual void SaveYAMLNode(YAML::Emitter& out) const override
        {
            AssetMetadata::SaveYAMLNode(out);
            
            out << YAML::Key << "Index" << YAML::Value << Index;
        }
    };

    struct MaterialAssetMetadata : public AssetMetadata
    {
        uint32_t Index = 0;

        virtual void LoadYAMLNode(const YAML::Node& node) override
        {
            AssetMetadata::LoadYAMLNode(node);
            
            if (node["Index"])
                Index = node["Index"].as<uint32_t>();
        }

        virtual void SaveYAMLNode(YAML::Emitter& out) const override
        {
            AssetMetadata::SaveYAMLNode(out);
            
            out << YAML::Key << "Index" << YAML::Value << Index;
        }
    };

    class Asset
    {
        public:
        UUID Handle;
        AssetType GetType() { return m_Type; };

        protected:
        bool m_Loaded = false;
        AssetType m_Type;
    };
}
