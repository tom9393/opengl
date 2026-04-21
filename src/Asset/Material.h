#include "Texture.h"
#include "Shader.h"
#include "MaterialSpecification.h"
#include "DefaultTextures.h"

namespace Engine {

    enum class BlendMode { Opaque, Transparent, Additive };

    class Material : public Asset
    {
        public:
        Shared<Shader> TargetShader;

        BlendMode mode = BlendMode::Opaque;
        bool DepthWrite = true;
        bool DoubleSided = false;

        void Add(MaterialProperty& property)
        {
            if (property.Type == PropertyKey::Map) {
                // If it's a map, unpack it and store it in the texture map
                const auto& texMap = std::get<TextureMap>(property.Value);
                m_Textures[texMap.Type] = texMap.MapTexture;
            } else {
                // Otherwise, store it in the standard properties map
                m_Properties[property.Type] = property.Value;
            }
        }

        void Bind() const
        {
            TargetShader->Bind();

            for (const auto& [key, value] : m_Properties)
            {
                std::string uniformName = GetUniformNameForProperty(key);
                if (std::holds_alternative<float>(value))
                    TargetShader->SetFloat(uniformName, std::get<float>(value));
                else if (std::holds_alternative<int>(value))
                    TargetShader->SetInt(uniformName, std::get<int>(value));
                else if (std::holds_alternative<glm::vec4>(value))
                    TargetShader->SetVec4(uniformName, std::get<glm::vec4>(value));
            }

            BindTexture(TextureType::Albedo,    "pbr.albedoMap",    0, DefaultTextures::GetWhiteTexture());
            BindTexture(TextureType::Normal,    "pbr.normalMap",    1, DefaultTextures::GetNormalTexture());
            BindTexture(TextureType::Metallic,  "pbr.metallicMap",  2, DefaultTextures::GetWhiteTexture());
            BindTexture(TextureType::Roughness, "pbr.roughnessMap", 3, DefaultTextures::GetWhiteTexture());
            BindTexture(TextureType::AO,        "pbr.aoMap",        4, DefaultTextures::GetWhiteTexture());
        }

        //private:
        std::string GetUniformNameForProperty(PropertyKey key) const {
            switch(key) {
                case PropertyKey::AlbedoTint:          return "pbr.albedoTint";
                case PropertyKey::Emission:            return "pbr.emission";
                case PropertyKey::RoughnessMultiplier: return "pbr.roughnessMultiplier";
                case PropertyKey::MetallicMultiplier:  return "pbr.metallicMultiplier";
                default: return "";
            }
        }

        // Helper to bind a texture if it exists, or a default fallback if it doesn't
        void BindTexture(TextureType type, const std::string& uniformName, uint32_t slot, Shared<Texture2D> fallback) const {
            auto it = m_Textures.find(type);
            Shared<Texture2D> texToBind = (it != m_Textures.end() && it->second) ? it->second : fallback;
            
            texToBind->Bind(slot);
            TargetShader->SetInt(uniformName, slot);
        }

        std::unordered_map<PropertyKey, PropertyValue> m_Properties;
        std::unordered_map<TextureType, Shared<Texture2D>> m_Textures;
    };
}