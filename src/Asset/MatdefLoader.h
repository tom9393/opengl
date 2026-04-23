#pragma once

#include "../pch.h"
#include "Material.h"

namespace Engine {

    static TextureType StringToTextureType(const std::string& typeStr)
    {
        if (typeStr == "Albedo")    return TextureType::Albedo;
        if (typeStr == "Normal")    return TextureType::Normal;
        if (typeStr == "Metallic")  return TextureType::Metallic;
        if (typeStr == "Roughness") return TextureType::Roughness;
        if (typeStr == "AO")        return TextureType::AO;
        return TextureType::None;
    }

    static PropertyKey StringToPropertyKey(const std::string& typeStr)
    {
        if (typeStr == "AlbedoTint")          return PropertyKey::AlbedoTint;
        if (typeStr == "Emission")            return PropertyKey::Emission;
        if (typeStr == "RoughnessMultiplier") return PropertyKey::RoughnessMultiplier;
        if (typeStr == "MetallicMultiplier")  return PropertyKey::MetallicMultiplier;
        
        return PropertyKey::Map; 
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
                std::cout << "[Material Loader] YAML Error: " << e.what() << std::endl;
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

            auto texBindings = matDef["TextureBindings"];
            if (texBindings) {
                for (auto it = texBindings.begin(); it != texBindings.end(); ++it) {
                    TextureType type = StringToTextureType(it->first.as<std::string>());
                    material->TextureBindings[type] = it->second.as<std::string>();
                }
            }

            auto propBindings = matDef["PropertyBindings"];
            if (propBindings) {
                for (auto it = propBindings.begin(); it != propBindings.end(); ++it) {
                    PropertyKey key = StringToPropertyKey(it->first.as<std::string>());
                    material->PropertyBindings[key] = it->second.as<std::string>();
                }
            }

            return material;
        }
    };

}