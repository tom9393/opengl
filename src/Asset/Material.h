#pragma once

#include "MaterialSpecification.h"
#include "DefaultTextures.h"
#include "Shader.h"
#include "Texture.h"

namespace Engine {

    enum class BlendMode { Opaque, Transparent, Additive };

    class Material : public Asset
    {
        public:
        Shared<Shader> TargetShader;

        BlendMode mode = BlendMode::Opaque;
        bool DepthWrite = true;
        bool DoubleSided = false;

        std::unordered_map<MaterialType, PropertyValue> m_Properties;
        std::unordered_map<MaterialType, std::string> m_PropertyBindings;
        std::unordered_map<MaterialType, std::string> m_TextureBindings;

        void Add(const MaterialProperty& property)
        {
            m_Properties[property.Type] = property.Value;
        }

        void Remove(const MaterialType key)
        {
            m_Properties.erase(key);
        }

        void Bind() const
        {
            TargetShader->Bind();

            for (const auto& [key, value] : m_Properties)
            {
                auto bindingIt = m_PropertyBindings.find(key);
                if (bindingIt == m_PropertyBindings.end()) continue;

                const std::string& uniformName = bindingIt->second;

                if (std::holds_alternative<float>(value)) 
                    TargetShader->SetFloat(uniformName, std::get<float>(value));
                else if (std::holds_alternative<int>(value)) 
                    TargetShader->SetInt(uniformName, std::get<int>(value));
                else if (std::holds_alternative<glm::vec4>(value)) 
                    TargetShader->SetVec4(uniformName, std::get<glm::vec4>(value));
            }

            uint32_t slot = 0;
            for (const auto& [key, uniformName] : m_TextureBindings)
            {
                auto it = m_Properties.find(key);
                Shared<Texture2D> texture = nullptr;

                if (it != m_Properties.end() && std::holds_alternative<Shared<Texture2D>>(it->second)) {
                    texture = std::get<Shared<Texture2D>>(it->second);
                }

                if (!texture) {
                    texture = GetDefaultFallback(key);
                }

                texture->Bind(slot);
                TargetShader->SetInt(uniformName, slot);
                slot++;
            }
        }

        Shared<Texture2D> GetDefaultFallback(MaterialType type) const 
        {
            switch(type) {
                case MaterialType::Normal:   return DefaultTextures::GetNormalTexture();
                case MaterialType::Metallic: return DefaultTextures::GetBlackTexture();
                default:                     return DefaultTextures::GetWhiteTexture();
            }
        }
    };
}