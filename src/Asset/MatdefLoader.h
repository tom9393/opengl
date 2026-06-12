#pragma once

#include <yaml-cpp/yaml.h>

#include "pch.h"
#include "Material.h"

namespace Engine {

    static MaterialType StringToMaterialType(const std::string& typeStr)
    {
        if (typeStr == "Albedo")               return MaterialType::Albedo;
        if (typeStr == "Normal")               return MaterialType::Normal;
        if (typeStr == "Metallic")             return MaterialType::Metallic;
        if (typeStr == "Roughness")            return MaterialType::Roughness;
        if (typeStr == "AO")                   return MaterialType::AO;
        if (typeStr == "Height")               return MaterialType::Height;
        if (typeStr == "Emissive")             return MaterialType::Emissive;
        
        if (typeStr == "Diffuse")              return MaterialType::Diffuse;
        if (typeStr == "Specular")             return MaterialType::Specular;
        
        if (typeStr == "Transparency")         return MaterialType::Transparency;
        if (typeStr == "AlbedoTint")           return MaterialType::AlbedoTint;
        if (typeStr == "Emission")             return MaterialType::Emission;
        if (typeStr == "RoughnessMultiplier")  return MaterialType::RoughnessMultiplier;
        if (typeStr == "MetallicMultiplier")   return MaterialType::MetallicMultiplier;

        return MaterialType::None;
    }

    class MatdefLoader
    {
    public:
    static Shared<Material> LoadMaterial(const std::string& filepath)
        {
            YAML::Node data;
            try {
                data = YAML::LoadFile(filepath);
            } catch (const YAML::ParserException& e) {
                std::cerr << "Material loader YAML error: " << e.what() << std::endl;
                return nullptr;
            }

            auto matDef = data["MaterialDefinition"];
            if (!matDef) return nullptr;

            Shared<Material> material = MakeShared<Material>();

            if (matDef["Shader"]) {
                material->TargetShader = ShaderLibrary::Get(matDef["Shader"].as<std::string>());
            }

            if (matDef["BlendMode"]) {
                std::string mode = matDef["BlendMode"].as<std::string>();
                if (mode == "Transparent") material->mode = BlendMode::Transparent;
                else if (mode == "Additive") material->mode = BlendMode::Additive;
            }

            auto bindings = matDef["Bindings"];
            if (bindings) {
                for (auto it = bindings.begin(); it != bindings.end(); ++it) {
                    MaterialType type = StringToMaterialType(it->first.as<std::string>());
                    material->m_Bindings[type] = it->second.as<std::string>();
                }
            }

            return material;
        }
    };

}