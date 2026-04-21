#include "Texture.h"
#include "Shader.h"
#include "MaterialSpecification.h"
#include "DefaultTextures.h"

namespace Engine {

    enum class BlendMode { Opaque, Transparent, Additive };

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

    class Material : public Asset
    {
        public:
        Shared<Shader> TargetShader;

        BlendMode mode = BlendMode::Opaque;
        bool DepthWrite = true;
        bool DoubleSided = false;

        void Add(const MaterialProperty& property)
        {
            if (property.Type == PropertyKey::Map) {
                const auto& texMap = std::get<TextureMap>(property.Value);
                m_Textures[texMap.Type] = texMap.MapTexture;
            } else {
                m_Properties[property.Type] = property.Value;
            }
        }

        void LoadMaterialDefinition(const std::string& filepath)
        {
            YAML::Node data;
            try {
                data = YAML::LoadFile(filepath);
            } catch (YAML::ParserException& e) {
                std::cout << "Failed to load material definition: " << e.what() << std::endl;
                return;
            }

            auto matDef = data["MaterialDefinition"];
            if (!matDef) return;

            TargetShader = ShaderLibrary::Get(matDef["Shader"].as<std::string>());

            auto texBindings = matDef["TextureBindings"];
            if (texBindings)
            {
                for (auto it = texBindings.begin(); it != texBindings.end(); ++it)
                {
                    TextureType enumType = StringToTextureType(it->first.as<std::string>());
                    TextureBindings[enumType] = it->second.as<std::string>();
                }
            }

            auto propBindings = matDef["PropertyBindings"];
            if (propBindings)
            {
                for (auto it = propBindings.begin(); it != propBindings.end(); ++it)
                {
                    PropertyKey enumType = StringToPropertyKey(it->first.as<std::string>());
                    PropertyBindings[enumType] = it->second.as<std::string>();
                }
            }
        }

        void Bind() const
        {
            TargetShader->Bind();

            for (const auto& [key, value] : m_Properties)
            {
                auto bindingIt = PropertyBindings.find(key);
                if (bindingIt == PropertyBindings.end()) continue;

                std::string uniformName = bindingIt->second;

                if (std::holds_alternative<float>(value))
                    TargetShader->SetFloat(uniformName, std::get<float>(value));
                else if (std::holds_alternative<int>(value))
                    TargetShader->SetInt(uniformName, std::get<int>(value));
                else if (std::holds_alternative<glm::vec4>(value))
                    TargetShader->SetVec4(uniformName, std::get<glm::vec4>(value));
            }

            uint32_t slot = 0;
            for (const auto& [type, uniformName] : TextureBindings)
            {
                BindTexture(type, uniformName, slot);
                slot++;
            }
        }

        //private:
        // Helper to bind a texture if it exists, or a default fallback if it doesn't
        void BindTexture(TextureType type, const std::string& uniformName, uint32_t slot) const {
            auto it = m_Textures.find(type);
            Shared<Texture2D> texToBind = (it != m_Textures.end() && it->second) ? it->second : GetDefaultFallback(type);
            texToBind->Bind(slot);
            TargetShader->SetInt(uniformName, slot);
        }

        Shared<Texture2D> GetDefaultFallback(TextureType type) const {
            switch(type) {
                case TextureType::Normal: return DefaultTextures::GetNormalTexture();
                case TextureType::Metallic: return DefaultTextures::GetBlackTexture();
                default: return DefaultTextures::GetWhiteTexture();
            }
        }

        std::unordered_map<PropertyKey, PropertyValue> m_Properties;
        std::unordered_map<TextureType, Shared<Texture2D>> m_Textures;

        std::unordered_map<TextureType, std::string> TextureBindings;
        std::unordered_map<PropertyKey, std::string> PropertyBindings;
    };
}