#pragma once

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

        void Add(const MaterialProperty& property)
        {
            if (property.Type == PropertyKey::Map) {
                const auto& texMap = std::get<TextureMap>(property.Value);
                m_Textures[texMap.Type] = texMap.MapTexture;
            } else {
                m_Properties[property.Type] = property.Value;
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

                if (std::holds_alternative<float>(value)) TargetShader->SetFloat(uniformName, std::get<float>(value));
                else if (std::holds_alternative<int>(value)) TargetShader->SetInt(uniformName, std::get<int>(value));
                else if (std::holds_alternative<glm::vec4>(value)) TargetShader->SetVec4(uniformName, std::get<glm::vec4>(value));
            }

            uint32_t slot = 0;
            for (const auto& [type, uniformName] : TextureBindings)
            {
                auto it = m_Textures.find(type);
                Shared<Texture2D> texture = (it != m_Textures.end() && it->second) ? it->second : GetDefaultFallback(type);
                texture->Bind(slot);
                TargetShader->SetInt(uniformName, slot);
                slot++;
            }
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